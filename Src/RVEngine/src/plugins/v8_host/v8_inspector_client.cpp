#include "v8_inspector_client.hpp"
#include "v8_engine.hpp"
#include <v8-inspector.h>
#include <sstream>
#include <algorithm>
#include <chrono>
#include <cstring>

// For SHA-1 used in WebSocket handshake
#include <wincrypt.h>
#pragma comment(lib, "crypt32.lib")
#pragma comment(lib, "advapi32.lib")

namespace v8_host {

// ============================================================================
// Utility: SHA-1 hash (Windows CryptoAPI)
// ============================================================================

static std::vector<uint8_t> sha1_hash(const std::string& input) {
    HCRYPTPROV hProv = 0;
    HCRYPTHASH hHash = 0;
    std::vector<uint8_t> result(20, 0);

    if (!CryptAcquireContext(&hProv, nullptr, nullptr, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT))
        return result;

    if (!CryptCreateHash(hProv, CALG_SHA1, 0, 0, &hHash)) {
        CryptReleaseContext(hProv, 0);
        return result;
    }

    CryptHashData(hHash, (const BYTE*)input.data(), (DWORD)input.size(), 0);
    DWORD hash_len = 20;
    CryptGetHashParam(hHash, HP_HASHVAL, result.data(), &hash_len, 0);

    CryptDestroyHash(hHash);
    CryptReleaseContext(hProv, 0);
    return result;
}

// ============================================================================
// Utility: Base64 encode
// ============================================================================

static std::string base64_encode(const std::vector<uint8_t>& data) {
    DWORD encoded_len = 0;
    CryptBinaryToStringA(data.data(), (DWORD)data.size(),
        CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF, nullptr, &encoded_len);
    std::string result(encoded_len, 0);
    CryptBinaryToStringA(data.data(), (DWORD)data.size(),
        CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF, &result[0], &encoded_len);
    // Trim trailing null/whitespace
    while (!result.empty() && (result.back() == 0 || result.back() == '\r' || result.back() == '\n'))
        result.pop_back();
    return result;
}

// ============================================================================
// V8Inspector StringView helpers
// ============================================================================

static std::string stringViewToUtf8(const v8_inspector::StringView& view) {
    if (view.is8Bit()) {
        return std::string(reinterpret_cast<const char*>(view.characters8()), view.length());
    } else {
        // Convert UTF-16 to UTF-8
        std::string result;
        result.reserve(view.length());
        for (size_t i = 0; i < view.length(); i++) {
            uint16_t ch = view.characters16()[i];
            if (ch < 0x80) {
                result += static_cast<char>(ch);
            } else if (ch < 0x800) {
                result += static_cast<char>(0xC0 | (ch >> 6));
                result += static_cast<char>(0x80 | (ch & 0x3F));
            } else {
                result += static_cast<char>(0xE0 | (ch >> 12));
                result += static_cast<char>(0x80 | ((ch >> 6) & 0x3F));
                result += static_cast<char>(0x80 | (ch & 0x3F));
            }
        }
        return result;
    }
}

// ============================================================================
// Constructor / Destructor
// ============================================================================

V8InspectorClientImpl::V8InspectorClientImpl(
    v8::Isolate* isolate, v8::Platform* platform, int port, bool wait_for_connection)
    : m_isolate(isolate), m_platform(platform), m_port(port)
{
    // Initialize Winsock
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    // Create inspector
    m_inspector = v8_inspector::V8Inspector::create(isolate, this);

    // Start TCP server
    if (!startServer(port)) {
        V8Engine::log_error("Failed to start inspector server on port " + std::to_string(port));
        return;
    }

    V8Engine::log_info("V8 Inspector listening on ws://127.0.0.1:" + std::to_string(port));
    V8Engine::log_info("Open in Chrome: devtools://devtools/bundled/inspector.html?v8only=true&ws=127.0.0.1:" + std::to_string(port));

    m_break_on_start = wait_for_connection;

    if (wait_for_connection) {
        V8Engine::log_info("Waiting for debugger to connect...");
        while (!m_connected) {
            if (acceptConnection()) {
                V8Engine::log_info("Debugger connected!");
            }
            Sleep(100);
        }
        // Create session for V8 Inspector
        createSession();
        // Drain initial CDP handshake (Runtime.enable, Debugger.enable, etc.)
        // Multiple rounds since VS Code sends in bursts
        for (int round = 0; round < 10; round++) {
            Sleep(100);
            drainMessages();
        }
        V8Engine::log_info("Initial CDP handshake complete");
    }
}

V8InspectorClientImpl::~V8InspectorClientImpl() {
    contextDestroyed();
    m_session.reset();
    m_inspector.reset();
    closeConnection();
    stopServer();
    WSACleanup();
}

// ============================================================================
// Context Management
// ============================================================================

void V8InspectorClientImpl::contextCreated(v8::Local<v8::Context> context, const std::string& name) {
    m_context.Reset(m_isolate, context);

    v8_inspector::StringView context_name(
        reinterpret_cast<const uint8_t*>(name.c_str()), name.size());
    v8_inspector::V8ContextInfo info(context, 1, context_name);
    m_inspector->contextCreated(info);
}

void V8InspectorClientImpl::contextDestroyed() {
    if (!m_context.IsEmpty()) {
        m_inspector->contextDestroyed(m_context.Get(m_isolate));
        m_context.Reset();
    }
}

// ============================================================================
// Message Pump
// ============================================================================

void V8InspectorClientImpl::pumpMessages() {
    // Try to accept new connection if not connected
    if (!m_connected) {
        if (acceptConnection()) {
            // New connection - create session and drain initial CDP handshake
            createSession();
            for (int round = 0; round < 5; round++) {
                Sleep(50);
                drainMessages();
            }
        }
        return;
    }

    // Read and dispatch ALL pending CDP messages (not just one)
    for (int i = 0; i < 50; i++) {
        // Check if there's data in TCP socket OR already in our recv buffer
        bool has_buffered_data = !m_recv_buffer.empty();
        bool has_socket_data = false;

        if (!has_buffered_data) {
            fd_set readfds;
            FD_ZERO(&readfds);
            FD_SET(m_client_socket, &readfds);
            timeval tv = { 0, 0 }; // non-blocking
            int ready = select(0, &readfds, nullptr, nullptr, &tv);
            has_socket_data = (ready > 0 && FD_ISSET(m_client_socket, &readfds));
        }

        if (has_buffered_data || has_socket_data) {
            std::string msg = readWebSocketFrame();
            if (!msg.empty()) {
                dispatchProtocolMessage(msg);
            } else {
                break;
            }
        } else {
            break;
        }
    }
}

void V8InspectorClientImpl::runMessageLoopOnPause(int contextGroupId) {
    m_paused = true;
    while (m_paused && m_connected) {
        // Check buffer first, then wait on socket
        bool has_buffered_data = !m_recv_buffer.empty();
        bool has_socket_data = false;

        if (!has_buffered_data) {
            fd_set readfds;
            FD_ZERO(&readfds);
            FD_SET(m_client_socket, &readfds);
            timeval tv = { 0, 50000 }; // 50ms timeout
            int ready = select(0, &readfds, nullptr, nullptr, &tv);
            has_socket_data = (ready > 0 && FD_ISSET(m_client_socket, &readfds));
        }

        if (has_buffered_data || has_socket_data) {
            std::string msg = readWebSocketFrame();
            if (!msg.empty()) {
                dispatchProtocolMessage(msg);
            }
        }

        // Run V8 platform tasks
        while (v8::platform::PumpMessageLoop(m_platform, m_isolate)) {}
    }
}

void V8InspectorClientImpl::quitMessageLoopOnPause() {
    m_paused = false;
}

v8::Local<v8::Context> V8InspectorClientImpl::ensureDefaultContextInGroup(int contextGroupId) {
    return m_context.Get(m_isolate);
}

double V8InspectorClientImpl::currentTimeMS() {
    return std::chrono::duration<double, std::milli>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
}

// ============================================================================
// V8Inspector::Channel
// ============================================================================

void V8InspectorClientImpl::sendResponse(int callId,
    std::unique_ptr<v8_inspector::StringBuffer> message) {
    sendMessageToFrontend(message->string());
}

void V8InspectorClientImpl::sendNotification(
    std::unique_ptr<v8_inspector::StringBuffer> message) {
    sendMessageToFrontend(message->string());
}

void V8InspectorClientImpl::flushProtocolNotifications() {
    // Nothing to flush in our synchronous implementation
}

void V8InspectorClientImpl::sendMessageToFrontend(const v8_inspector::StringView& message) {
    if (!m_connected) return;
    std::string json = stringViewToUtf8(message);
    if (!sendWebSocketFrame(json)) {
        V8Engine::log_error("Failed to send inspector message, disconnecting");
        closeConnection();
    }
}

void V8InspectorClientImpl::sendConsoleMessage(v8::Local<v8::Context> context,
    const std::string& type, const std::string& message) {
    // Console messages are automatically captured by V8 Inspector
    // when console API is installed via inspector->contextCreated().
    // This method is kept as a hook for C++ -> DevTools console logging.
    if (!m_connected || !m_session) return;

    // Build and send a Runtime.consoleAPICalled CDP notification manually
    std::string inner = "{\"method\":\"Runtime.consoleAPICalled\",\"params\":{";
    inner += "\"type\":\"" + type + "\",";
    inner += "\"args\":[{\"type\":\"string\",\"value\":\"";
    // Escape the message for JSON
    for (char c : message) {
        if (c == '"') inner += "\\\"";
        else if (c == '\\') inner += "\\\\";
        else if (c == '\n') inner += "\\n";
        else if (c == '\r') inner += "\\r";
        else if (c == '\t') inner += "\\t";
        else inner += c;
    }
    inner += "\"}],";
    inner += "\"executionContextId\":1,";
    inner += "\"timestamp\":" + std::to_string(currentTimeMS());
    inner += "}}";

    sendWebSocketFrame(inner);
}

void V8InspectorClientImpl::createSession() {
    if (m_session) return;
    v8_inspector::StringView state;
    m_session = m_inspector->connect(1, this, state,
        v8_inspector::V8Inspector::ClientTrustLevel::kFullyTrusted);
    V8Engine::log_info("Inspector session created");
}

void V8InspectorClientImpl::dispatchProtocolMessage(const std::string& message) {
    // Forward all CDP messages directly to V8 Inspector
    // (js-debug adds its own Target.* multiplexing layer internally,
    //  we must NOT add another one — just be a flat CDP server)
    if (!m_session) {
        createSession();
    }

    v8_inspector::StringView msg_view(
        reinterpret_cast<const uint8_t*>(message.c_str()), message.size());
    m_session->dispatchProtocolMessage(msg_view);
}

void V8InspectorClientImpl::scheduleBreakOnNextStatement(const std::string& reason) {
    if (!m_session) return;
    v8_inspector::StringView reason_view(
        reinterpret_cast<const uint8_t*>(reason.c_str()), reason.size());
    m_session->schedulePauseOnNextStatement(reason_view, v8_inspector::StringView());
    V8Engine::log_info("Scheduled break on next statement: " + reason);
}

void V8InspectorClientImpl::drainMessages() {
    if (!m_connected) return;
    // Read all pending CDP messages (VS Code sends a burst on connect)
    for (int i = 0; i < 100; i++) {
        // Check buffer first, then socket
        bool has_buffered_data = !m_recv_buffer.empty();
        bool has_socket_data = false;

        if (!has_buffered_data) {
            fd_set readfds;
            FD_ZERO(&readfds);
            FD_SET(m_client_socket, &readfds);
            timeval tv = { 0, 10000 }; // 10ms
            int ready = select(0, &readfds, nullptr, nullptr, &tv);
            has_socket_data = (ready > 0 && FD_ISSET(m_client_socket, &readfds));
        }

        if (has_buffered_data || has_socket_data) {
            std::string msg = readWebSocketFrame();
            if (!msg.empty()) {
                dispatchProtocolMessage(msg);
            } else {
                break;
            }
        } else {
            break;
        }
    }
}

// ============================================================================
// TCP Server
// ============================================================================

bool V8InspectorClientImpl::startServer(int port) {
    m_server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (m_server_socket == INVALID_SOCKET) return false;

    // Allow reuse
    int opt = 1;
    setsockopt(m_server_socket, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));

    sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port = htons((u_short)port);

    if (bind(m_server_socket, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        closesocket(m_server_socket);
        m_server_socket = INVALID_SOCKET;
        return false;
    }

    if (listen(m_server_socket, 1) == SOCKET_ERROR) {
        closesocket(m_server_socket);
        m_server_socket = INVALID_SOCKET;
        return false;
    }

    // Set non-blocking
    u_long mode = 1;
    ioctlsocket(m_server_socket, FIONBIO, &mode);

    return true;
}

void V8InspectorClientImpl::stopServer() {
    if (m_server_socket != INVALID_SOCKET) {
        closesocket(m_server_socket);
        m_server_socket = INVALID_SOCKET;
    }
}

bool V8InspectorClientImpl::acceptConnection() {
    if (m_server_socket == INVALID_SOCKET || m_connected) return false;

    sockaddr_in client_addr;
    int addr_len = sizeof(client_addr);
    SOCKET client = accept(m_server_socket, (sockaddr*)&client_addr, &addr_len);
    if (client == INVALID_SOCKET) return false;

    m_client_socket = client;

    // Perform WebSocket handshake
    if (!performWebSocketHandshake()) {
        closesocket(m_client_socket);
        m_client_socket = INVALID_SOCKET;
        return false;
    }

    // Set non-blocking after handshake
    u_long mode = 1;
    ioctlsocket(m_client_socket, FIONBIO, &mode);

    m_connected = true;
    V8Engine::log_info("Inspector WebSocket connection established");

    return true;
}

void V8InspectorClientImpl::closeConnection() {
    if (m_client_socket != INVALID_SOCKET) {
        closesocket(m_client_socket);
        m_client_socket = INVALID_SOCKET;
    }
    m_connected = false;
    m_recv_buffer.clear();
    m_session.reset();
    V8Engine::log_info("Inspector disconnected, ready for reconnect");
}

// ============================================================================
// WebSocket Handshake
// ============================================================================

bool V8InspectorClientImpl::performWebSocketHandshake() {
    // Read HTTP upgrade request
    char buf[4096];
    int received = recv(m_client_socket, buf, sizeof(buf) - 1, 0);
    if (received <= 0) return false;
    buf[received] = 0;

    std::string request(buf);

    // Handle HTTP requests (discovery endpoints for VS Code / Chrome)
    bool is_websocket = (request.find("Upgrade: websocket") != std::string::npos) ||
                        (request.find("Upgrade: Websocket") != std::string::npos) ||
                        (request.find("Upgrade: WebSocket") != std::string::npos);

    if (!is_websocket) {
        if (request.find("GET /json/version") != std::string::npos) {
            std::string json_response =
                "{\"Browser\":\"V8Host/1.0\","
                "\"Protocol-Version\":\"1.3\","
                "\"V8-Version\":\"" + std::string(v8::V8::GetVersion()) + "\","
                "\"webSocketDebuggerUrl\":\"ws://127.0.0.1:" + std::to_string(m_port) + "\""
                "}";

            std::string http_response =
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: application/json; charset=UTF-8\r\n"
                "Content-Length: " + std::to_string(json_response.size()) + "\r\n"
                "Connection: close\r\n\r\n" + json_response;

            send(m_client_socket, http_response.c_str(), (int)http_response.size(), 0);
            return false;
        }
        if (request.find("GET /json") != std::string::npos) {
            std::string json_response =
                "[{\"description\":\"V8Host Arma 3\","
                "\"devtoolsFrontendUrl\":\"devtools://devtools/bundled/js_app.html?experiments=true&v8only=true&ws=127.0.0.1:" + std::to_string(m_port) + "\","
                "\"id\":\"v8host-1\","
                "\"title\":\"V8Host - Arma 3\","
                "\"type\":\"node\","
                "\"url\":\"file://\","
                "\"webSocketDebuggerUrl\":\"ws://127.0.0.1:" + std::to_string(m_port) + "\""
                "}]";

            std::string http_response =
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: application/json; charset=UTF-8\r\n"
                "Content-Length: " + std::to_string(json_response.size()) + "\r\n"
                "Connection: close\r\n\r\n" + json_response;

            send(m_client_socket, http_response.c_str(), (int)http_response.size(), 0);
            return false;
        }
    }

    // Extract Sec-WebSocket-Key
    std::string key_header = "Sec-WebSocket-Key: ";
    auto key_pos = request.find(key_header);
    if (key_pos == std::string::npos) return false;

    auto key_start = key_pos + key_header.size();
    auto key_end = request.find("\r\n", key_start);
    std::string ws_key = request.substr(key_start, key_end - key_start);

    // Compute accept value: SHA-1(key + magic GUID), then base64
    std::string magic = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
    auto hash = sha1_hash(ws_key + magic);
    std::string accept = base64_encode(hash);

    // Send upgrade response
    std::string response =
        "HTTP/1.1 101 Switching Protocols\r\n"
        "Upgrade: websocket\r\n"
        "Connection: Upgrade\r\n"
        "Sec-WebSocket-Accept: " + accept + "\r\n\r\n";

    send(m_client_socket, response.c_str(), (int)response.size(), 0);
    return true;
}

// ============================================================================
// WebSocket Framing (RFC 6455 minimal)
// ============================================================================

bool V8InspectorClientImpl::sendWebSocketFrame(const std::string& data) {
    if (m_client_socket == INVALID_SOCKET) return false;

    std::vector<uint8_t> frame;
    frame.push_back(0x81); // FIN + text opcode

    size_t len = data.size();
    if (len < 126) {
        frame.push_back(static_cast<uint8_t>(len));
    } else if (len < 65536) {
        frame.push_back(126);
        frame.push_back(static_cast<uint8_t>((len >> 8) & 0xFF));
        frame.push_back(static_cast<uint8_t>(len & 0xFF));
    } else {
        frame.push_back(127);
        for (int i = 7; i >= 0; i--) {
            frame.push_back(static_cast<uint8_t>((len >> (8 * i)) & 0xFF));
        }
    }

    frame.insert(frame.end(), data.begin(), data.end());

    int total_sent = 0;
    int to_send = static_cast<int>(frame.size());
    while (total_sent < to_send) {
        int sent = send(m_client_socket, (const char*)frame.data() + total_sent,
                       to_send - total_sent, 0);
        if (sent == SOCKET_ERROR) return false;
        total_sent += sent;
    }
    return true;
}

std::string V8InspectorClientImpl::readWebSocketFrame() {
    if (m_client_socket == INVALID_SOCKET) return "";

    // Try to read more data from socket (non-blocking)
    char buf[65536];
    int received = recv(m_client_socket, buf, sizeof(buf), 0);
    if (received > 0) {
        m_recv_buffer.append(buf, received);
    } else if (received == 0 || (received < 0 && WSAGetLastError() != WSAEWOULDBLOCK)) {
        V8Engine::log_info("Inspector connection closed");
        closeConnection();
        return "";
    }
    // Note: WOULDBLOCK is fine — we may still have data in m_recv_buffer

    // Parse WebSocket frame from buffer
    if (m_recv_buffer.size() < 2) return "";

    uint8_t byte0 = m_recv_buffer[0];
    uint8_t byte1 = m_recv_buffer[1];

    // Check for close frame
    if ((byte0 & 0x0F) == 0x08) {
        V8Engine::log_info("Inspector WebSocket close frame received");
        closeConnection();
        return "";
    }

    bool masked = (byte1 & 0x80) != 0;
    uint64_t payload_len = byte1 & 0x7F;
    size_t header_len = 2;

    if (payload_len == 126) {
        if (m_recv_buffer.size() < 4) return "";
        payload_len = ((uint16_t)(uint8_t)m_recv_buffer[2] << 8) |
                      (uint8_t)m_recv_buffer[3];
        header_len = 4;
    } else if (payload_len == 127) {
        if (m_recv_buffer.size() < 10) return "";
        payload_len = 0;
        for (int i = 0; i < 8; i++) {
            payload_len = (payload_len << 8) | (uint8_t)m_recv_buffer[2 + i];
        }
        header_len = 10;
    }

    size_t mask_offset = header_len;
    if (masked) header_len += 4;

    size_t total_frame_len = header_len + payload_len;
    if (m_recv_buffer.size() < total_frame_len) return ""; // incomplete

    // Unmask payload
    std::string payload(m_recv_buffer.begin() + header_len,
                        m_recv_buffer.begin() + total_frame_len);
    if (masked) {
        uint8_t mask[4];
        memcpy(mask, m_recv_buffer.data() + mask_offset, 4);
        for (size_t i = 0; i < payload.size(); i++) {
            payload[i] ^= mask[i % 4];
        }
    }

    // Remove consumed frame from buffer
    m_recv_buffer.erase(0, total_frame_len);

    return payload;
}

} // namespace v8_host
