import { randomBytes } from "crypto";
import { checkLength, range } from "../utils/jsUtils.js";
import {
  Bufferable,
  numToUInt32BE,
  serializeToBuffer,
} from "../wasm/serialize.js";

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

  toString() {
    return "0x" + this.buffer.toString("hex");
  }

  toBuffer() {
    return this.buffer;
  }
}

export class Fq {
  static SIZE_IN_BYTES = 32;
  static MAX_SIZE = 2 ** 256; // TODO: Check max size for an Fq

  private buffer: Buffer;

  constructor(input: Buffer | number) {
    if (Buffer.isBuffer(input)) {
      if (input.length != Fq.SIZE_IN_BYTES) {
        throw new Error(
          `Unexpected buffer size ${input.length} (expected ${Fr.SIZE_IN_BYTES} bytes)`
        );
      }
      this.buffer = input;
    } else {
      if (input >= Fq.MAX_SIZE) {
        throw new Error(
          `Input value ${input} too large (expected ${Fq.MAX_SIZE})`
        );
      }
      this.buffer = numToUInt32BE(input);
    }
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

  public publicInputs: Vector<Fr>;
  public proofWitnessIndices: Vector<UInt32>;

  constructor(
    public p0: AffineElement,
    public p1: AffineElement,
    publicInputsData: Fr[],
    proofWitnessIndicesData: UInt32[]
  ) {
    this.publicInputs = new Vector(publicInputsData);
    this.proofWitnessIndices = new Vector(proofWitnessIndicesData);
  }

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

export class Vector<T extends Bufferable> {
  constructor(public items: T[]) {}

  toBuffer() {
    return serializeToBuffer(this.items.length, this.items);
  }
}

export class UInt8Vector {
  constructor(public buffer: Buffer) {}

  toBuffer() {
    return serializeToBuffer(this.buffer.length, this.buffer);
  }
}

export type UInt32 = number;

// TODO: Define proper type for AztecAddress
export type AztecAddress = Fr;

/**
 * Affine element of G1, a group defined over Bn254
 * cpp/barretenberg/cpp/src/aztec/stdlib/types/native_types.hpp
 * cpp/barretenberg/cpp/src/aztec/ecc/curves/bn254/g1.hpp
 * cpp/barretenberg/cpp/src/aztec/ecc/groups/affine_element.hpp
 */
export class AffineElement {
  constructor(public x: Fq, public y: Fq) {}

  toBuffer() {
    return serializeToBuffer(this.x, this.y);
  }
}

// TODO: Adapt from cpp/barretenberg/cpp/src/aztec/proof_system/verification_key/verification_key.hpp
export type VK = Buffer;
