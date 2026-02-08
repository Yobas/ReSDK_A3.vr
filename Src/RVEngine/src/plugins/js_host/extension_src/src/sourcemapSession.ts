import { MappedPosition, SourceMapConsumer } from 'source-map';
import { LoggingDebugSession } from 'vscode-debugadapter';
import { SourcemapArguments } from "./sourcemapArguments";
const path = require('path');
const fs = require('fs');

/** Normalize path for case-insensitive comparison on Windows:
 *  - backslashes → forward slashes
 *  - lowercase drive letter (e.g. P: → p:)
 */
function normalizePath(p: string): string {
	let result = p.replace(/\\/g, '/');
	if (result.length >= 2 && result[1] === ':')
		result = result[0].toLowerCase() + result.substring(1);
	return result;
}

export abstract class SourcemapSession extends LoggingDebugSession {
	// a map of all absolute file sources found in the sourcemaps
	// SourceMapConsumer v0.6 types lack .file/.sources — use any
	private _fileToSourceMap = new Map<string, any>();
	private _sourceMapsLoading: Promise<any>|undefined;
	// keep track of the sourcemaps and the location of the file.map used to load it
	private _sourceMaps = new Map<any, string>();

	abstract async logTrace(message: string);
	abstract async getArguments(): Promise<SourcemapArguments>;

	async loadSourceMaps() {
		const commonArgs = await this.getArguments();
		if (this._sourceMapsLoading)
			return await this._sourceMapsLoading;

		let sourceMaps = Object.keys(commonArgs.sourceMaps || {}) || [];

		let promises = sourceMaps.map(sourcemap => (async () => {
			try {
				let json = JSON.parse(fs.readFileSync(sourcemap).toString());
				let smc: any = await new SourceMapConsumer(json);
				this._sourceMaps.set(smc, sourcemap);
				let sourceMapRoot = path.dirname(sourcemap);
				let sources = smc.sources.map(source => path.join(sourceMapRoot, source) as string);
				for (let source of sources) {
					const key = normalizePath(source);
					const other = this._fileToSourceMap.get(key);
					if (other) {
						this.logTrace(`sourcemap for ${source} found in ${other.file}.map and ${sourcemap}`);
					}
					else {
						this._fileToSourceMap.set(key, smc);
					}
				}
			}
			catch (e) {
				this.logTrace(`loadSourceMaps error for ${sourcemap}: ${e}`);
			}
		})());

		this._sourceMapsLoading = Promise.all(promises);

		return await this._sourceMapsLoading;
	}

	async translateFileToRemote(file: string): Promise<string> {
		await this.loadSourceMaps();

		const sm = this._fileToSourceMap.get(normalizePath(file));
		if (!sm)
			return file;
		return sm.file;
	}

	private async getRemoteAbsolutePath(remoteFile: string, remoteRoot?: string): Promise<string> {
		const commonArgs = await this.getArguments();
		if (remoteRoot == null)
			remoteRoot = commonArgs.remoteRoot;
		if (remoteRoot)
			remoteFile = path.join(remoteRoot, remoteFile);
		return remoteFile;
	}

	private async getRemoteRelativePath(remoteFile: string, remoteRoot?: string): Promise<string> {
		const commonArgs = await this.getArguments();
		if (remoteRoot == null)
			remoteRoot = commonArgs.remoteRoot;
		if (remoteRoot)
			return path.relative(remoteRoot, remoteFile);
		return remoteFile;
	}

	private async getLocalAbsolutePath(localFile: string): Promise<string> {
		const commonArgs = await this.getArguments();
		if (commonArgs.localRoot)
			return path.join(commonArgs.localRoot, localFile);
		return localFile;
	}
	private async getLocalRelativePath(localFile: string): Promise<string> {
		const commonArgs = await this.getArguments();
		if (commonArgs.localRoot)
			return path.relative(commonArgs.localRoot, localFile);
		return localFile;
	}


	async translateFileLocationToRemote(sourceLocation: MappedPosition): Promise<MappedPosition[]> {
		await this.loadSourceMaps();
		const commonArgs = await this.getArguments();

		// step 1: translate the absolute local source position to a relative source position.
		// (has sourcemap) /local/path/to/test.ts:10 -> test.js:15
		// (no sourcemap)  /local/path/to/test.js:10 -> test.js:10
		// step 2: translate the relative source file to an absolute remote source file
		// (has sourcemap) test.js:15 -> /remote/path/to/test.js:15
		// (no sourcemap)  test.js:10 -> /remote/path/to/test.js:10
		// (no remote root)test.js:10 -> test.js:10

		try {
			const sm = this._fileToSourceMap.get(normalizePath(sourceLocation.source));
			if (!sm)
				throw new Error('no source map');
			const sourcemap = this._sourceMaps.get(sm);
			if (!sourcemap)
				throw new Error();
			const actualSourceLocation = Object.assign({}, sourceLocation);
			// convert the local absolute path into a sourcemap relative path.
			actualSourceLocation.source = path.relative(path.dirname(sourcemap), sourceLocation.source).replace(/\\/g, '/');
			delete actualSourceLocation.column;
			this.logTrace(`translateFileLocationToRemote: querying sourcemap for source="${actualSourceLocation.source}" line=${actualSourceLocation.line}, sm.sources=${JSON.stringify(sm.sources)}`);
			// let unmappedPosition: NullablePosition = sm.generatedPositionFor(actualSourceLocation);
			let unmappedPositions = sm.allGeneratedPositionsFor(actualSourceLocation);
			this.logTrace(`translateFileLocationToRemote: allGeneratedPositionsFor returned ${JSON.stringify(unmappedPositions)}`);
			if (!unmappedPositions || !unmappedPositions.length)
				throw new Error('map failed');
			// now given a source mapped relative path, translate that into a remote path.
			const smp = this._sourceMaps.get(sm);
			let remoteRoot = commonArgs.sourceMaps && commonArgs.sourceMaps[smp!];
			// sm.file may be null if the source map lacks "file" field — derive from .map path
			const generatedFile = sm.file || (smp ? path.basename(smp).replace(/\.map$/, '') : null);
			if (!generatedFile)
				throw new Error('cannot determine generated file from source map');
			let remoteFile = await this.getRemoteAbsolutePath(generatedFile, remoteRoot);
			return unmappedPositions.map(unmappedPosition => ({
				source: remoteFile,
				line: (unmappedPosition.line || 0),
				column: unmappedPosition.column || 0,
			}));
			// return {
			// 	source: remoteFile,
			// 	// the source-map docs indicate that line is 1 based, but that seems to be wrong.
			// 	line: (unmappedPosition.line || 0) + 1,
			// 	column: unmappedPosition.column || 0,
			// };
		}
		catch (e) {
			this.logTrace(`translateFileLocationToRemote FAILED for ${sourceLocation.source}:${sourceLocation.line} — ${e}`);
			// local files need to be resolved to remote files.
			let ret = Object.assign({}, sourceLocation);
			ret.source = await this.getRemoteAbsolutePath(await this.getLocalRelativePath(sourceLocation.source));
			return [ret];
		}
	}

	async translateRemoteLocationToLocal(sourceLocation: MappedPosition): Promise<MappedPosition> {
		await this.loadSourceMaps();
		const commonArgs = await this.getArguments();

		try {
			for (let sm of this._sourceMaps.keys()) {
				const smp = this._sourceMaps.get(sm);

				// given a remote path, translate that into a source map relative path for lookup
				let remoteRoot = commonArgs.sourceMaps && commonArgs.sourceMaps[smp!];
				let relativeFile = await this.getRemoteRelativePath(sourceLocation.source, remoteRoot);

				this.logTrace(`translateRemoteLocationToLocal: relativeFile="${relativeFile}" sm.file="${sm.file}"`);
				if (relativeFile !== sm.file)
					continue;

				const queryPos = {
					column: sourceLocation.column,
					line: sourceLocation.line,
				};
				this.logTrace(`translateRemoteLocationToLocal: querying originalPositionFor(${JSON.stringify(queryPos)})`);
				const original = sm.originalPositionFor(queryPos);
				this.logTrace(`translateRemoteLocationToLocal: result = ${JSON.stringify(original)}`);
				if (original.line === null || original.column === null || original.source === null)
					throw new Error("unable to map");

				// now given a source mapped relative path, translate that into a local path.
				return {
					source: path.join(path.dirname(smp), original.source),
					line: original.line,
					column: original.column,
				};
			}
			throw new Error();
		}
		catch (e) {
			// remote files need to be resolved to local files.
			let ret = Object.assign({}, sourceLocation);
			ret.source = await this.getLocalAbsolutePath(await this.getRemoteRelativePath(sourceLocation.source));
			return ret;
		}
	}
}