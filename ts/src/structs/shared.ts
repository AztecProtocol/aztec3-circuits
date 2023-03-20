import { randomBytes } from "crypto";
import { serializeToBuffer } from "../wasm/serialize.js";
import { checkLength } from "./utils.js";

export class Fr {
  static SIZE_IN_BYTES = 32;

  constructor(private buffer: Buffer) {
    if (buffer.length != Fr.SIZE_IN_BYTES) {
      throw new Error(
        `Unexpected buffer size ${buffer.length} (expected ${Fr.SIZE_IN_BYTES} bytes)`
      );
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

export type UInt32 = number;

// TODO: Define proper type for AztecAddress
export type AztecAddress = Fr;

// TODO: What is a Curve::G1?
export type G1 = Fr;

export type Proof = Buffer;
export type VK = Buffer;
