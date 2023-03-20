import { Crs } from "../crs/index.js";
import { expectSerializeToMatchSnapshot } from "../tests/expectSerializeToMatchSnapshot.js";
import { makePreviousKernelData } from "../tests/factories.js";
import { CircuitsWasm } from "../wasm/circuits_wasm.js";

describe("structs/kernel", () => {
  it(`serializes and prints previous_kernel_data`, async () => {
    const wasm = await CircuitsWasm.new();
    const previousKernelData = makePreviousKernelData();
    const crs: Crs = new Crs(/*example, circuit size = 100*/ 100);
    await crs.init();
    wasm.call("abis__set_global_verifier_reference_string", crs.getG2Data);
    await expectSerializeToMatchSnapshot(
      previousKernelData.toBuffer(),
      "abis__test_roundtrip_serialize_previous_kernel_data",
      wasm
    );
  });
});
