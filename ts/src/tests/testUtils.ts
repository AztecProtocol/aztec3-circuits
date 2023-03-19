import { Fr } from "../structs/shared.js";

/**
 * Test only. Easy to identify big endian buffer serialize.
 * @param num - The number.
 * @param bufferSize - The buffer size.
 * @returns The buffer.
 */
export function asBEBuffer(num: number, bufferSize = 32) {
  const buf = Buffer.alloc(bufferSize);
  buf.writeUInt32BE(num, bufferSize - 4);
  return buf;
}
/**
 * Test only. Easy to identify big endian field serialize.
 * @param num - The number.
 * @param bufferSize - The buffer size.
 * @returns The buffer.
 */
export function fr(n: number) {
  return new Fr(asBEBuffer(n + 1));
}
