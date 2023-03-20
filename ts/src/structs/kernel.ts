import { serializeToBuffer } from "../wasm/serialize.js";
import {
  KERNEL_L1_MSG_STACK_LENGTH,
  KERNEL_NEW_COMMITMENTS_LENGTH,
  KERNEL_NEW_CONTRACTS_LENGTH,
  KERNEL_NEW_NULLIFIERS_LENGTH,
  KERNEL_PRIVATE_CALL_STACK_LENGTH,
  KERNEL_PUBLIC_CALL_STACK_LENGTH,
  KERNEL_OPTIONALLY_REVEALED_DATA_LENGTH,
  VK_TREE_HEIGHT,
} from "./constants.js";
import {
  AggregationObject,
  AztecAddress,
  EthAddress,
  Fr,
  Proof,
  UInt32,
  VK,
} from "./shared.js";
import { TxContext } from "./tx.js";
import { checkLength } from "./utils.js";

export class OldTreeRoots {
  constructor(
    public privateDataTreeRoot: Fr,
    public nullifierTreeRoot: Fr,
    public contractTreeRoot: Fr,
    public privateKernelVkTreeRoot: Fr // future enhancement
  ) {}

  toBuffer() {
    return serializeToBuffer(
      this.privateDataTreeRoot,
      this.nullifierTreeRoot,
      this.contractTreeRoot,
      this.privateKernelVkTreeRoot
    );
  }
}

export class ConstantData {
  constructor(public oldTreeRoots: OldTreeRoots, public txContext: TxContext) {}

  toBuffer() {
    return serializeToBuffer(this.oldTreeRoots, this.txContext);
  }
}

// Not to be confused with ContractDeploymentData (maybe think of better names)
export class NewContractData {
  constructor(
    public contractAddress: AztecAddress,
    public portalContractAddress: EthAddress,
    public functionTreeRoot: Fr
  ) {}

  toBuffer() {
    return serializeToBuffer(
      this.contractAddress,
      this.portalContractAddress,
      this.functionTreeRoot
    );
  }
}

export class AccumulatedData {
  constructor(
    public aggragationObject: AggregationObject, // Contains the aggregated proof of all previous kernel iterations

    public privateCallCount: Fr,

    public newCommitments: Fr[],
    public newNullifiers: Fr[],

    public privateCallStack: Fr[],
    public publicCallStack: Fr[],
    public l1MsgStack: Fr[],

    public newContracts: NewContractData[],

    public optionallyRevealedData: Fr[] // TODO
  ) {
    checkLength(
      this.newCommitments,
      KERNEL_NEW_COMMITMENTS_LENGTH,
      "newCommitments"
    );
    checkLength(
      this.newNullifiers,
      KERNEL_NEW_NULLIFIERS_LENGTH,
      "newNullifiers"
    );
    checkLength(
      this.privateCallStack,
      KERNEL_PRIVATE_CALL_STACK_LENGTH,
      "privateCallStack"
    );
    checkLength(
      this.publicCallStack,
      KERNEL_PUBLIC_CALL_STACK_LENGTH,
      "publicCallStack"
    );
    checkLength(this.l1MsgStack, KERNEL_L1_MSG_STACK_LENGTH, "l1MsgStack");
    checkLength(this.newContracts, KERNEL_NEW_CONTRACTS_LENGTH, "newContracts");
    checkLength(
      this.optionallyRevealedData,
      KERNEL_OPTIONALLY_REVEALED_DATA_LENGTH,
      "optionallyRevealedData"
    );
  }

  toBuffer() {
    return serializeToBuffer(
      this.aggragationObject,
      this.privateCallCount,
      ...this.newCommitments,
      ...this.newNullifiers,
      ...this.privateCallStack,
      ...this.publicCallStack,
      ...this.l1MsgStack,
      ...this.newContracts,
      ...this.optionallyRevealedData
    );
  }
}

export class PrivateKernelPublicInputs {
  constructor(
    public end: AccumulatedData,
    public constants: ConstantData,
    public isPrivateKernel: true
  ) {}

  toBuffer() {
    return serializeToBuffer(this.end, this.constants, this.isPrivateKernel);
  }
}

export class PreviousKernelData {
  constructor(
    public publicInputs: PrivateKernelPublicInputs,
    public proof: Proof,
    public vk: VK,
    public vkIndex: UInt32, // the index of the kernel circuit's vk in a big tree of kernel circuit vks
    public vkSiblingPath: Fr[]
  ) {
    checkLength(this.vkSiblingPath, VK_TREE_HEIGHT, "vkSiblingPath");
  }

  toBuffer() {
    return serializeToBuffer(
      this.publicInputs,
      this.proof,
      this.vk,
      this.vkIndex,
      ...this.vkSiblingPath
    );
  }
}
