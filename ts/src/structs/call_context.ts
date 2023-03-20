import { serializeToBuffer } from "../wasm/serialize.js";
import { AztecAddress, EthAddress } from "./shared.js";

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
      this.portalContractAddress.toBuffer(),
      this.isDelegateCall,
      this.isStaticCall,
      this.isContractDeployment
    );
  }
}
