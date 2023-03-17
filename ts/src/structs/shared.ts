export type Fr = Buffer;
export type UInt32 = number;
export type Proof = Buffer;
export type VK = Buffer;
export type AztecAddress = Buffer;
export type EthAddress = Buffer;
export type AggregationObject = Buffer;

export type MembershipWitness = { 
  leafIndex: UInt32, // type tbd
  siblingPath: Fr[]
};