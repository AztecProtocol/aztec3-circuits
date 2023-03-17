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
    return this.buffer;
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

export type UInt32 = number;
export type Proof = Buffer;
export type VK = Buffer;
export type AztecAddress = Buffer;
export type AggregationObject = Buffer;
