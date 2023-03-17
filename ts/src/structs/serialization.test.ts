import { CircuitsWasm } from "../wasm/circuits_wasm.js";
import { uint8ArrayToNum } from "../wasm/serialize.js";
import { Fr } from "./shared.js";
import { ContractDeploymentData, TxContext } from "./tx.js";

// /**
//  * Call context.
//  * @see abis/call_context.hpp
//  */
// class CallContext {
//   constructor(
//     public msgSender: AztecAddress,
//     public storageContractAddress: AztecAddress,
//     public portalContractAddress: EthAddress,
//     public isDelegateCall: boolean,
//     public isStaticCall: boolean,
//     public isContractDeployment: boolean
//   ) {}
//   toBuffer(): Buffer {
//     return serializeToBuffer(
//       this.msgSender,
//       this.storageContractAddress,
//       this.portalContractAddress,
//       this.isDelegateCall,
//       this.isStaticCall,
//       this.isContractDeployment
//     );
//   }
// }

// class PrivateCircuitPublicInputs {
//   constructor(
//     public callContext: CallContext,
//     public args: Fr[],
//     public returnValues: Fr[],
//     public newCommitments: Fr[],
//     public newNullifiers: Fr[],
//     public privateCallStack: Fr[],
//     public publicCallStack: Fr[],
//     public l1MsgStack: Fr[],
//     public historicPrivateDataTreeRoot: Fr
//   ) {}
//   toBuffer(): Buffer {
//     return serializeToBuffer(
//       this.callContext,
//       this.args,
//       this.returnValues,
//       this.newCommitments,
//       this.isStaticCall,
//       this.isContractDeployment
//     );
//   }
// }

function txContext() {
  const deploymentData = new ContractDeploymentData(
    Fr.random(),
    Fr.random(),
    Fr.random(),
    Fr.random(),
    Fr.random()
  );

  return new TxContext(false, false, true, deploymentData);
}

describe("basic struct serialization", () => {
  const wasm: CircuitsWasm = new CircuitsWasm();
  beforeAll(async () => {
    await wasm.init();
  });
  for (const { name, object, method } of [
    // {
    //   name: "PrivateCircuitPublicInputs",
    //   object: new PrivateCircuitPublicInputs(),
    //   method: "abis__test_roundtrip_serialize_private_circuits_public_inputs",
    // },
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
      // Get a passthrough trivial copy of our buffer
      // We sanity check this for basic serialization checks
      // Note that without structured output from the C++,
      // we can be tricked by output that happens to be the same length.
      // NOTE: bbmalloc's memory.
      const outputBufPtr = wasm.call(method, inputBufPtr, outputBufSizePtr);
      // Read the size pointer
      const outputBufSize = uint8ArrayToNum(
        wasm.getMemorySlice(outputBufSizePtr, outputBufSizePtr + 4)
      );
      const outputBuf = wasm.getMemorySlice(
        outputBufPtr,
        outputBufPtr + outputBufSize
      );
      expect(inputBuf).toEqual(Buffer.from(outputBuf));
      // Free memory
      wasm.call("bbfree", outputBufPtr);
      wasm.call("bbfree", outputBufSizePtr);
      wasm.call("bbfree", inputBufPtr);
    });
  }
});
