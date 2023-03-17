import { assertLength, range } from "../utils/jsUtils.js";
import { CircuitsWasm } from "../wasm/circuits_wasm.js";
import {
  numToUInt32LE,
  serializeToBuffer,
  uint8ArrayToNum,
} from "../wasm/serialize.js";
import {
  ARGS_LENGTH,
  L1_MSG_STACK_LENGTH,
  NEW_COMMITMENTS_LENGTH,
  NEW_NULLIFIERS_LENGTH,
  PRIVATE_CALL_STACK_LENGTH,
  PUBLIC_CALL_STACK_LENGTH,
  RETURN_VALUES_LENGTH,
} from "./constants.js";
import { AztecAddress, EthAddress, Fr } from "./shared.js";
import { ContractDeploymentData, TxContext } from "./tx.js";

/**
 * Call context.
 * @see abis/call_context.hpp
 */
export class CallContext {
  constructor(
    public msgSender: AztecAddress,
    public storageContractAddress: AztecAddress,
    public portalContractAddress: EthAddress,
    public isDelegateCall: boolean,
    public isStaticCall: boolean,
    public isContractDeployment: boolean
  ) {}
  toBuffer(): Buffer {
    return serializeToBuffer(
      this.msgSender,
      this.storageContractAddress,
      this.portalContractAddress,
      this.isDelegateCall,
      this.isStaticCall,
      this.isContractDeployment
    );
  }
}

export class PrivateCircuitPublicInputs {
  constructor(
    public callContext: CallContext,
    public args: Fr[],
    public returnValues: Fr[],
    public newCommitments: Fr[],
    public newNullifiers: Fr[],
    public privateCallStack: Fr[],
    public publicCallStack: Fr[],
    public l1MsgStack: Fr[],
    public historicPrivateDataTreeRoot: Fr
  ) {
    assertLength(this, "args", ARGS_LENGTH);
    assertLength(this, "returnValues", RETURN_VALUES_LENGTH);
    assertLength(this, "newCommitments", NEW_COMMITMENTS_LENGTH);
    assertLength(this, "newNullifiers", NEW_NULLIFIERS_LENGTH);
    assertLength(this, "privateCallStack", PRIVATE_CALL_STACK_LENGTH);
    assertLength(this, "publicCallStack", PUBLIC_CALL_STACK_LENGTH);
    assertLength(this, "l1MsgStack", L1_MSG_STACK_LENGTH);
  }
  toBuffer(): Buffer {
    return serializeToBuffer(
      this.callContext,
      this.args,
      this.returnValues,
      this.newCommitments,
      this.newNullifiers,
      this.privateCallStack,
      this.publicCallStack,
      this.l1MsgStack,
      this.historicPrivateDataTreeRoot
    );
  }
}

function fr(n: number) {
  return new Fr(numToUInt32LE(n));
}

function privateCircuitPublicInputs() {
  return new PrivateCircuitPublicInputs(
    new CallContext(
      numToUInt32LE(1),
      numToUInt32LE(2),
      new EthAddress(numToUInt32LE(3)),
      true,
      true,
      true
    ),
    range(ARGS_LENGTH).map(fr),
    range(RETURN_VALUES_LENGTH).map(fr),
    range(NEW_COMMITMENTS_LENGTH).map(fr),
    range(NEW_NULLIFIERS_LENGTH).map(fr),
    range(PRIVATE_CALL_STACK_LENGTH).map(fr),
    range(PUBLIC_CALL_STACK_LENGTH).map(fr),
    range(L1_MSG_STACK_LENGTH).map(fr),
    new Fr(numToUInt32LE(1))
  );
}

function txContext() {
  const deploymentData = new ContractDeploymentData(
    new Fr(numToUInt32LE(1)),
    new Fr(numToUInt32LE(2)),
    new Fr(numToUInt32LE(3)),
    new Fr(numToUInt32LE(4)),
    new Fr(numToUInt32LE(5))
  );
  return new TxContext(false, false, true, deploymentData);
}

describe("basic struct serialization", () => {
  const wasm: CircuitsWasm = new CircuitsWasm();
  beforeAll(async () => {
    await wasm.init();
  });
  for (const { name, object, method } of [
    {
      name: "PrivateCircuitPublicInputs",
      object: privateCircuitPublicInputs(),
      method: "abis__test_roundtrip_serialize_private_circuits_public_inputs",
    },
    {
      name: "TxContext",
      object: txContext(),
      method: "abis__test_roundtrip_serialize_tx_context",
    },
  ]) {
    it(`serializes a ${name} and calls trivial C++ code`, () => {
      const inputBuf = object.toBuffer();
      const inputBufPtr = wasm.call("bbmalloc", inputBuf.length);
      wasm.writeMemory(inputBufPtr, inputBuf);
      const outputBufSizePtr = wasm.call("bbmalloc", 4);
      // Get a string version of our object. As a quick and dirty test,
      // we compare a snapshot of its string form to its previous form.
      const outputBufPtr = wasm.call(method, inputBufPtr, outputBufSizePtr);
      // Read the size pointer
      const outputBufSize = uint8ArrayToNum(
        wasm.getMemorySlice(outputBufSizePtr, outputBufSizePtr + 4)
      );
      const outputBuf = wasm.getMemorySlice(
        outputBufPtr,
        outputBufPtr + outputBufSize
      );
      const outputStr = Buffer.from(outputBuf).toString("utf-8");
      expect(outputStr).toMatchSnapshot();
      // Free memory
      wasm.call("bbfree", outputBufPtr);
      wasm.call("bbfree", outputBufSizePtr);
      wasm.call("bbfree", inputBufPtr);
    });
  }
});
