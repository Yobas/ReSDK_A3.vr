#pragma once
#include <v8.h>
#include <v8-inspector.h>
#include <libplatform/libplatform.h>
#include <string>
#include <vector>
#include <memory>
#include <winsock2.h>
#include <ws2tcpip.h>

namespace v8_host {

/**
 * Minimal V8 Inspector implementation using raw TCP + CDP (Chrome DevTools Protocol).
 *
 * Architecture:
 *   - TCP server listens on configured port
 *   - Accepts ONE WebSocket connection (Chrome DevTools or VS Code)
 *   - Bridges CDP JSON messages between the WebSocket and V8 Inspector
 *
 * For simplicity in Phase 1, we use a basic WebSocket implementation
 * without external libraries (libwebsockets would be better but adds deps).
 * Instead, we implement minimal WebSocket framing over raw TCP.
 */
class V8InspectorClientImpl : public v8_inspector::V8InspectorClient,
                               public v8_inspector::V8Inspector::Channel {
public:
    V8InspectorClientImpl(v8::Isolate* isolate, v8::Platform* platform,
                          int port, bool wait_for_connection);
    ~V8InspectorClientImpl();

    // Context management
    void contextCreated(v8::Local<v8::Context> context, const std::string& name);
    void contextDestroyed();

    // Call from game loop to process pending messages
    void pumpMessages();

    // Schedule break before next JS execution
    void scheduleBreakOnNextStatement(const std::string& reason = "debugger");

    // Process all pending incoming CDP messages (called during initial handshake)
    void drainMessages();

    // Send console message to Inspector (shows in DevTools Console)
    void sendConsoleMessage(v8::Local<v8::Context> context,
        const std::string& type, const std::string& message);

    // V8InspectorClient overrides
    void runMessageLoopOnPause(int contextGroupId) override;
    void quitMessageLoopOnPause() override;
    v8::Local<v8::Context> ensureDefaultContextInGroup(int contextGroupId) override;
    double currentTimeMS() override;

    // V8Inspector::Channel overrides
    void sendResponse(int callId,
        std::unique_ptr<v8_inspector::StringBuffer> message) override;
    void sendNotification(
        std::unique_ptr<v8_inspector::StringBuffer> message) override;
    void flushProtocolNotifications() override;

private:
    // TCP server management
    bool startServer(int port);
    void stopServer();
    bool acceptConnection();
    void closeConnection();

    // WebSocket handshake & framing
    bool performWebSocketHandshake();
    bool sendWebSocketFrame(const std::string& data);
    std::string readWebSocketFrame();

    // Send CDP message to client
    void sendMessageToFrontend(const v8_inspector::StringView& message);

    // Process one incoming CDP message
    void dispatchProtocolMessage(const std::string& message);

    v8::Isolate* m_isolate;
    v8::Platform* m_platform;
    v8::Global<v8::Context> m_context;

    std::unique_ptr<v8_inspector::V8Inspector> m_inspector;
    std::unique_ptr<v8_inspector::V8InspectorSession> m_session;

    // Create session immediately (needed for breakpoints to work)
    void createSession();

    // TCP state
    SOCKET m_server_socket = INVALID_SOCKET;
    SOCKET m_client_socket = INVALID_SOCKET;
    bool m_connected = false;
    bool m_paused = false;
    bool m_break_on_start = false;
    int m_port = 0;

    // Buffered incoming data
    std::string m_recv_buffer;
};

} // namespace v8_host
