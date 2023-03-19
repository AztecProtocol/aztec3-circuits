import { serializeToBuffer } from "../wasm/serialize.js";

/**
 * @see abis/function_data.hpp
 */
export class FunctionData {
  constructor(
    public functionSelector: number,
    public isPrivate: true,
    public isConstructor: true
  ) {}
  toBuffer(): Buffer {
    return serializeToBuffer(
      this.functionSelector,
      this.isPrivate,
      this.isConstructor
    );
  }
}
