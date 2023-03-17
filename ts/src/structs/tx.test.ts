import { dedent } from "ts-dedent";
import { CircuitsWasm } from "../wasm/circuits_wasm.js";
import { Fr } from "./shared.js";
import { ContractDeploymentData, TxContext } from "./tx.js";

describe("structs/tx", () => {
  const wasm: CircuitsWasm = new CircuitsWasm();

  beforeAll(async () => {
    await wasm.init();
  });

  it("serializes a tx to call into wasm", () => {
    const deploymentData = new ContractDeploymentData(
      Fr.random(),
      Fr.random(),
      Fr.random(),
      Fr.random(),
      Fr.random()
    );

    const txContext = new TxContext(false, false, true, deploymentData);

    const retPtr = wasm.call("bbmalloc", 1024);
    const txContextPtr = wasm.call("bbmalloc", 1024);
    wasm.writeMemory(txContextPtr, txContext.toBuffer());

    const retSize = wasm.call("abis__inspect_tx_context", txContextPtr, retPtr);
    const result = Buffer.from(
      wasm.getMemorySlice(retPtr, retPtr + retSize)
    ).toString();

    const expected = dedent`
      is_fee_payment_tx: 0
      is_rebate_payment_tx: 0
      is_contract_deployment_tx: 1
      contract_deployment_data: 
      contract_data_hash: ${deploymentData.contractDataHash}
      function_tree_root: ${deploymentData.functionTreeRoot}
      constructor_hash: ${deploymentData.constructorHash}
      contract_address_salt: ${deploymentData.contractAddressSalt}
      portal_contract_address: ${deploymentData.portalContractAddress}\n
    `;
    expect(result).toEqual(expected);

    wasm.call("bbfree", txContextPtr);
    wasm.call("bbfree", retPtr);
  });
});

// function bufferTo0xHex(buf: Buffer): string {
//   return "0x" + buf.toString("hex");
// }
