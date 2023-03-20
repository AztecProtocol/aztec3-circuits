import { expectSerializeToMatchSnapshot } from "../tests/expectSerializeToMatchSnapshot.js";
import { makePreviousKernelData } from "../tests/factories.js";

describe("structs/kernel", () => {
  it(`serializes and prints previous_kernel_data`, async () => {
    const previousKernelData = makePreviousKernelData();
    await expectSerializeToMatchSnapshot(
      previousKernelData.toBuffer(),
      "abis__test_roundtrip_serialize_previous_kernel_data"
    );
  });
});
