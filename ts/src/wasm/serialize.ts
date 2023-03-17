/**
 * For serializing an array of fixed length buffers.
 * TODO move to foundation pkg.
 * @param arr - Array of bufffers.
 * @returns The serialized buffers.
 */
export function serializeBufferArrayToVector(arr: Buffer[]) {
  const lengthBuf = Buffer.alloc(4);
  lengthBuf.writeUInt32BE(arr.length, 0);
  return Buffer.concat([lengthBuf, ...arr]);
}

/**
 * Helper function for deserializeArrayFromVector.
 */
type DeserializeFn<T> = (
  buf: Buffer,
  offset: number
) => {
  /**
   * The deserialized type.
   */
  elem: T;
  /**
   * How many bytes to advance by.
   */
  adv: number;
};

/**
 * For deserializing numbers to 32-bit little-endian form.
 * TODO move to foundation pkg.
 * @param n - The number.
 * @returns The endian-corrected number.
 */
export function deserializeArrayFromVector<T>(
  deserialize: DeserializeFn<T>,
  vector: Buffer,
  offset = 0
) {
  let pos = offset;
  const size = vector.readUInt32BE(pos);
  pos += 4;
  const arr = new Array<T>(size);
  for (let i = 0; i < size; ++i) {
    const { elem, adv } = deserialize(vector, pos);
    pos += adv;
    arr[i] = elem;
  }
  return { elem: arr, adv: pos - offset };
}

/**
 * For serializing numbers to 32 bit little-endian form.
 * TODO move to foundation pkg.
 * @param n - The number.
 * @returns The endian-corrected number.
 */
export function numToUInt32LE(n: number, bufferSize = 4) {
  const buf = Buffer.alloc(bufferSize);
  buf.writeUInt32LE(n, bufferSize - 4);
  return buf;
}

/**
 * For serializing booleans in structs for calling into wasm
 * @param bool value to serialize
 */
export function boolToBuffer(value: boolean) {
  return Buffer.from([value ? 1 : 0]);
}

/**
 * Deserialize the 256-bit number at address `offset`.
 * @param buf - The buffer.
 * @param offset - The address.
 * @returns The derserialized 256-bit field.
 */
export function deserializeField(buf: Buffer, offset = 0) {
  const adv = 32;
  return { elem: buf.slice(offset, offset + adv), adv };
}

/**
 * Serializes a list of objects contiguously for calling into wasm.
 * @param objs objects to serialize.
 * @returns a single buffer with the concatenation of all fields.
 */
export function serializeToBuffer(
  ...objs: (boolean | Buffer | { toBuffer: () => Buffer })[]
): Buffer {
  return Buffer.concat(
    objs.map((obj) => {
      if (Buffer.isBuffer(obj)) {
        return obj;
      } else if (typeof obj === "boolean") {
        return boolToBuffer(obj);
      } else {
        return obj.toBuffer();
      }
    })
  );
}
