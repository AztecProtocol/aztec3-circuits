import { expectSerializeToMatchSnapshot } from "../tests/expectSerializeToMatchSnapshot.js";
import { FunctionData } from "./function_data.js";

describe("basic FunctionData serialization", () => {
  it(`serializes a trivial FunctionData and prints it`, async () => {
    // Test the data case: writing (mostly) sequential numbers
    for (let i = 0; i < 16; i++) {
      await expectSerializeToMatchSnapshot(
        new FunctionData(1, true, true).toBuffer(),
        "abis__test_roundtrip_serialize_function_data"
      );
    }
  });
});
