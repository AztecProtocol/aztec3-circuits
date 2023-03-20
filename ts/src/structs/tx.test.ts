import { expectSerializeToMatchSnapshot } from "../tests/expectSerializeToMatchSnapshot.js";
import { fr } from "../tests/testUtils.js";
import { ContractDeploymentData, TxContext } from "./tx.js";

describe("structs/tx", () => {
  it(`serializes and prints object`, async () => {
    const deploymentData = new ContractDeploymentData(
      fr(1),
      fr(2),
      fr(3),
      fr(4)
    );

    const txContext = new TxContext(false, false, true, deploymentData);
    await expectSerializeToMatchSnapshot(
      txContext.toBuffer(),
      "abis__test_roundtrip_serialize_tx_context"
    );
  });
});
