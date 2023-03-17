import { serializeToBuffer } from "../wasm/serialize.js";
import { Fr } from "./shared.js";

/**
 * Contract deployment data in a @TxContext.
 * cpp/src/aztec3/circuits/abis/contract_deployment_data.hpp
 * 
 * @todo Used the Cpp type as source of truth, which does not match the specification.
 * Spec includes constructorVkHash but does not have contract_data_hash nor constructor_hash.
 */
export class ContractDeploymentData {
  constructor (
    public contractDataHash: Fr,
    public functionTreeRoot: Fr,
    public constructorHash: Fr,
    public contractAddressSalt: Fr,
    /**
     * EthAddress of the Portal Contract
     * TODO: Change to EthAddress once there's a uint160 circuit type
     */
    public portalContractAddress: Fr,
  ) { }

  toBuffer() {
    return serializeToBuffer(
      this.contractDataHash,
      this.functionTreeRoot,
      this.constructorHash,
      this.contractAddressSalt,
      this.portalContractAddress,
    );
  }
}

/**
 * Transaction context
 * cpp/src/aztec3/circuits/abis/tx_context.hpp
 */
export class TxContext {
  constructor (
    public isFeePaymentTx: false,
    public isRebatePaymentTx: false,
    public isContractDeployment: true,
    public contractDeploymentData: ContractDeploymentData,
    public referenceBlockNumber: Fr,
  ) {}

  toBuffer() {
    return serializeToBuffer(
      this.isFeePaymentTx,
      this.isRebatePaymentTx,
      this.isContractDeployment,
      this.contractDeploymentData,
      this.referenceBlockNumber,
    );
  }
}