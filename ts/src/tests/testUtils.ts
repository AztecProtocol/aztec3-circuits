import { Fr } from "../structs/shared.js";
import { numToUInt32BE } from "../wasm/serialize.js";

/**
 * Test only. Easy to identify big endian field serialize.
 * @param num - The number.
 * @param bufferSize - The buffer size.
 * @returns The buffer.
 */
export function fr(n: number) {
  return new Fr(numToUInt32BE(n, 32));
}
