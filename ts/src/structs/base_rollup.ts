import { PreviousKernelData } from "./kernel.js";
import { AggregationObject, Fr, MembershipWitness, UInt32 } from "./shared.js";

export type NullifierLeafPreimage = {
  leafValue: Fr;
  nextValue: Fr;
  nextIndex: UInt32;
};

export type AppendOnlyTreeSnapshot = {
  root: Fr;
  nextAvailableLeafIndex: UInt32;
};

export type ConstantBaseRollupData = {
  // The very latest roots as at the very beginning of the entire rollup:
  startTreeOfHistoricPrivateDataTreeRootsSnapshot: AppendOnlyTreeSnapshot;
  startTreeOfHistoricContractTreeRootsSnapshot: AppendOnlyTreeSnapshot;
  treeOfHistoricL1ToL2MsgTreeRootsSnapshot: AppendOnlyTreeSnapshot;

  // Some members of this struct tbd:
  privateKernelVkTreeRoot: Fr;
  publicKernelVkTreeRoot: Fr;
  baseRollupVkHash: Fr;
  mergeRollupVkHash: Fr;
};

export type BaseRollupInputs = {
  kernelData: [PreviousKernelData, PreviousKernelData];

  startNullifierTreeSnapshot: AppendOnlyTreeSnapshot;
  lowNullifierLeafPreimages: NullifierLeafPreimage[];
  lowNullifierMembershipWitness: MembershipWitness[];

  oldPrivateDataTreeRootMembershipWitnesses: MembershipWitness[];
  oldContractsTreeRootMembershipWitnesses: MembershipWitness[];
  oldL1ToL2MsgTreeRootMembershipWitnesses: MembershipWitness[];

  constants: ConstantBaseRollupData;

  proverId: Fr;
};

enum RollupTypes {
  Base,
  Rollup,
}

export type BaseRollupPublicInputs = {
  rollupType: RollupTypes.Base;

  endAggregationObject: AggregationObject;

  // TODO: This was defined as ConstantRollupData, I'm assuming it's ConstantBaseRollupData
  constants: ConstantBaseRollupData;

  // The only tree root actually updated in this circuit is the nullifier tree, because earlier leaves (of low_nullifiers) must be updated to point to the new nullifiers in this circuit.
  startNullifierTreeSnapshot: AppendOnlyTreeSnapshot;
  endNullifierTreeSnapshots: AppendOnlyTreeSnapshot;

  newCommitmentsSubtreeRoot: Fr;
  newNullifiersSubtreeRoot: Fr;
  newContractLeavesSubtreeRoot: Fr;

  // Hashes (probably sha256) to make public inputs constant-sized (to then be unpacked on-chain)
  newCommitmentsHash: Fr;
  newNullifiersHash: Fr;
  newL1MsgsHash: Fr;
  newContractDataHash: Fr;
  proverContributionsHash: Fr;
};
