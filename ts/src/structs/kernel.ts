import { AggregationObject, AztecAddress, EthAddress, Fr, Proof, UInt32, VK } from "./shared.js";
import { TxContext } from "./tx.js";

export type OldTreeRoots = {
  privateDataTreeRoot: Fr,
  nullifierTreeRoot: Fr,
  contractTreeRoot: Fr,
  privateKernelVkTreeRoot: Fr, // future enhancement
};

export type ConstantData = {
  oldTreeRoots: OldTreeRoots,
  txContext: TxContext,
};

// Not to be confused with ContractDeploymentData (maybe think of better names)
export type NewContractData = {
  contractAddress: AztecAddress,
  portalContractAddress: EthAddress,
  functionTreeRoot: Fr,
};

export type AccumulatedData = {
  aggragationObject: AggregationObject, // Contains the aggregated proof of all previous kernel iterations
  
  privateCallCount: Fr,
  
  newCommitments: Fr[],
  newNullifiers: Fr[],
  
  privateCallStack: Fr[],
  publicCallStack: Fr[],
  l1MsgStack: Fr[],
  
  newContracts: NewContractData[],
  
  optionallyRevealedData: Fr[], // TODO
};

export type PrivateKernelPublicInputs = { 
  end: AccumulatedData,
  constants: ConstantData,
  isPrivateKernel: true,
};

export type PreviousKernelData = {
  publicInputs: PrivateKernelPublicInputs,
  proof: Proof,
  vk: VK,
  vkIndex: UInt32, // the index of the kernel circuit's vk in a big tree of kernel circuit vks
  vkSiblingPath: Fr[],
}