import { boolToBuffer } from "../wasm/serialize.js"
import { EthAddress, Fr } from "./shared.js"

export class ContractDeploymentData {
  constructor (
    public constructorVkHash: Fr, // the user needs to sign over the constructor!
    public functionTreeRoot: Fr, // the root of the function tree (see diagram)
    public contractAddressSalt: Fr, // random
    public portalContractAddress: EthAddress,
    public hidePrivateFunctionData: true,
  ) { }

  toBuffer() {
    return Buffer.concat([
      this.constructorVkHash,
      this.functionTreeRoot,
      this.contractAddressSalt,
      this.portalContractAddress,
      boolToBuffer(this.hidePrivateFunctionData),
    ])
  }
}

export class TxContext {
  constructor (
    public isFeePaymentTx: false,
    public isRebatePaymentTx: false,
    public isContractDeployment: true,
    public contractDeploymentData: ContractDeploymentData,
    public referenceBlockNumber: Fr,
  ) {}

  toBuffer() {
    return Buffer.concat([
      boolToBuffer(this.isFeePaymentTx),
      boolToBuffer(this.isRebatePaymentTx),
      boolToBuffer(this.isContractDeployment),
      this.contractDeploymentData.toBuffer(),
      this.referenceBlockNumber,
    ]);
  }
}