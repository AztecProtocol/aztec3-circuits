import { randomBytes } from "crypto";
import { checkLength, range } from "../utils/jsUtils.js";
import { numToUInt32BE, serializeToBuffer } from "../wasm/serialize.js";

export class Fr {
  static SIZE_IN_BYTES = 32;
  static CURVE_PRIME =
    0x30644e72e131a029b85045b68181585d2833e84879b9709143e1f593f0000001n;

  private buffer: Buffer;

  constructor(input: Buffer | number) {
    if (Buffer.isBuffer(input)) {
      if (input.length != Fr.SIZE_IN_BYTES) {
        throw new Error(
          `Unexpected buffer size ${input.length} (expected ${Fr.SIZE_IN_BYTES} bytes)`
        );
      }
      this.buffer = input;
    } else {
      if (input >= Fr.CURVE_PRIME) {
        throw new Error(
          `Input value ${input} too large (expected ${Fr.CURVE_PRIME})`
        );
      }
      this.buffer = numToUInt32BE(input);
    }
  }

  static random() {
    return new Fr(
      Buffer.concat([Buffer.alloc(1, 0), randomBytes(Fr.SIZE_IN_BYTES - 1)])
    );
  }

  toString() {
    return "0x" + this.buffer.toString("hex");
  }

  toBuffer() {
    return this.buffer;
  }
}

/**
 * For Ethereum addresses, which must be treated as 32 bytes.
 * @param buffer - The 20 byte ethereum address buffer.
 * @returns The 32 byte padded buffer.
 */
function pad32(buffer: Buffer) {
  // Create a 32-byte Buffer filled with zeros
  const paddedBuffer = Buffer.alloc(32);

  // Calculate the padding length
  const paddingLength = paddedBuffer.length - buffer.length;

  // Copy the original Buffer into the padded Buffer with an offset
  buffer.copy(paddedBuffer, paddingLength);
  return paddedBuffer;
}

export class EthAddress {
  static SIZE_IN_BYTES = 20;

  constructor(private buffer: Buffer) {
    if (buffer.length != EthAddress.SIZE_IN_BYTES) {
      throw new Error(
        `Unexpected buffer size ${buffer.length} (expected ${EthAddress.SIZE_IN_BYTES} bytes)`
      );
    }
  }

  static random() {
    return new Fr(randomBytes(EthAddress.SIZE_IN_BYTES));
  }

  toString() {
    return "0x" + this.buffer.toString("hex");
  }

  toBuffer() {
    return pad32(this.buffer);
  }
}

export class MembershipWitness<N extends number> {
  constructor(pathSize: N, public leafIndex: UInt32, public siblingPath: Fr[]) {
    checkLength(this.siblingPath, pathSize, "MembershipWitness.siblingPath");
  }

  toBuffer() {
    return serializeToBuffer(this.leafIndex, ...this.siblingPath);
  }

  static mock(size: number, start: number) {
    return new MembershipWitness(
      size,
      start,
      range(size, start).map((x) => new Fr(numToUInt32BE(x, 32)))
    );
  }
}

export class AggregationObject {
  public hasData: false = false;
  constructor(
    public p0: G1,
    public p1: G1,
    public publicInputs: Fr[],
    public proofWitnessIndices: UInt32[]
  ) {}

  toBuffer() {
    return serializeToBuffer(
      this.p0,
      this.p1,
      this.publicInputs,
      this.proofWitnessIndices,
      this.hasData
    );
  }
}

export class DynamicSizeBuffer {
  constructor(public buffer: Buffer) {}

  toBuffer() {
    return serializeToBuffer(this.buffer.length, this.buffer);
  }
}

export type UInt32 = number;

// TODO: Define proper type for AztecAddress
export type AztecAddress = Fr;

// TODO: What is a Curve::G1?
export type G1 = Fr;

// TODO: Adapt from cpp/barretenberg/cpp/src/aztec/proof_system/verification_key/verification_key.hpp
export type VK = Buffer;
