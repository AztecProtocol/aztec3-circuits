import { EMITTED_EVENTS_LENGTH } from "../structs/constants.js";
import { FunctionData } from "../structs/function_data.js";
import { AccumulatedData, ConstantData, NewContractData, OldTreeRoots, OptionallyRevealedData, PreviousKernelData, PrivateKernelPublicInputs } from "../structs/kernel.js";
import { AggregationObject, DynamicSizeBuffer, EthAddress, Fr, MembershipWitness } from "../structs/shared.js";
import { ContractDeploymentData, TxContext } from "../structs/tx.js";
import { range } from "../utils/jsUtils.js";
import { numToUInt32BE } from "../wasm/serialize.js";
import { KERNEL_L1_MSG_STACK_LENGTH, KERNEL_NEW_COMMITMENTS_LENGTH, KERNEL_NEW_CONTRACTS_LENGTH, KERNEL_NEW_NULLIFIERS_LENGTH, KERNEL_OPTIONALLY_REVEALED_DATA_LENGTH, KERNEL_PRIVATE_CALL_STACK_LENGTH, KERNEL_PUBLIC_CALL_STACK_LENGTH, VK_TREE_HEIGHT } from "./constants.js";

export function makeTxContext(seed: number): TxContext {
  const deploymentData = new ContractDeploymentData(fr(seed), fr(seed+1), fr(seed+2), fr(seed+3));
  return new TxContext(false, false, true, deploymentData);
}

export function makeOldTreeRoots(seed: number): OldTreeRoots {
  return new OldTreeRoots(
    fr(seed),
    fr(seed + 1),
    fr(seed + 2),
    fr(seed + 3),
  );
}

export function makeConstantData(seed: number = 1): ConstantData {
  return new ConstantData(makeOldTreeRoots(seed), makeTxContext(seed + 4));
}

export function makeAccumulatedData(seed: number = 1): AccumulatedData {
  return new AccumulatedData(
    makeAggregationObject(seed),
    fr(seed + 12),
    range(KERNEL_NEW_COMMITMENTS_LENGTH, seed + 0x100).map(fr),
    range(KERNEL_NEW_NULLIFIERS_LENGTH, seed + 0x200).map(fr),
    range(KERNEL_PRIVATE_CALL_STACK_LENGTH, seed + 0x300).map(fr),
    range(KERNEL_PUBLIC_CALL_STACK_LENGTH, seed + 0x400).map(fr),
    range(KERNEL_L1_MSG_STACK_LENGTH, seed + 0x500).map(fr),
    range(KERNEL_NEW_CONTRACTS_LENGTH, seed + 0x600).map(makeNewContractData),
    range(KERNEL_OPTIONALLY_REVEALED_DATA_LENGTH, seed + 0x700).map(makeOptionallyRevealedData),
  );
}

export function makeNewContractData(seed: number = 1): NewContractData {
  return new NewContractData(fr(seed), new EthAddress(fr(seed+1).toBuffer()), fr(seed+2));
}

export function makeOptionallyRevealedData(seed: number = 1): OptionallyRevealedData {
  return new OptionallyRevealedData(
    new Fr(seed),
    new FunctionData(seed + 1, true, true),
    range(EMITTED_EVENTS_LENGTH, seed + 0x100).map(x => new Fr(x)),
    new Fr(seed + 2),
    new EthAddress(new Fr(seed + 3).toBuffer()),
    true,
    false,
    true,
    false,
  );
}

export function makeAggregationObject(seed: number = 1): AggregationObject {
  return new AggregationObject(
    fr(seed), fr(seed + 1), range(4, seed + 2).map(fr), range(6, seed + 6),
  )
}

export function makePrivateKernelPublicInputs(seed: number = 1): PrivateKernelPublicInputs {
  return new PrivateKernelPublicInputs(makeAccumulatedData(seed), makeConstantData(seed + 0x100), true);
}

export function makeDynamicSizeBuffer(size: number, fill: number) {
  return new DynamicSizeBuffer(Buffer.alloc(size, fill));
}

export function makeMembershipWitness<N extends number>(size: number, start: number): MembershipWitness<N> {
  return new MembershipWitness(
    size,
    start,
    range(size, start).map(fr)
  );
}

export function makePreviousKernelData(seed: number = 1): PreviousKernelData {
  return new PreviousKernelData(
    makePrivateKernelPublicInputs(seed),
    makeDynamicSizeBuffer(16, seed + 0x80),
    Buffer.alloc(16, 0xcd),
    0x42,
    range(VK_TREE_HEIGHT, 0x1000).map(fr),
  );
}

/**
 * Test only. Easy to identify big endian field serialize.
 * @param num - The number.
 * @param bufferSize - The buffer size.
 * @returns The buffer.
 */
export function fr(n: number) {
  return new Fr(numToUInt32BE(n, 32));
}
