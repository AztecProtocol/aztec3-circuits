import { expectReserializeToMatchObject, expectSerializeToMatchSnapshot } from '../tests/expectSerialize.js';
import {
  fr,
  makeAppendOnlyTreeSnapshot,
  makeRootRollupPublicInputs,
  makePreviousKernelData,
  makePreviousRollupData,
} from '../tests/factories.js';
import { range } from '../utils/jsUtils.js';
import { CircuitsWasm } from '../wasm/circuits_wasm.js';
import { RootRollupInputs, RootRollupPublicInputs } from './root_rollup.js';
import {
  CONTRACT_TREE_HEIGHT,
  CONTRACT_TREE_ROOTS_TREE_HEIGHT,
  KERNEL_NEW_NULLIFIERS_LENGTH,
  NULLIFIER_TREE_HEIGHT,
  PRIVATE_DATA_TREE_HEIGHT,
  PRIVATE_DATA_TREE_ROOTS_TREE_HEIGHT,
} from './constants.js';
import { PreviousKernelData } from './kernel.js';
import { MembershipWitness } from './shared.js';
import { PreviousRollupData } from './merge_rollup.js';

describe('structs/root_rollup', () => {
  it(`serializes and prints RootRollupInputs`, async () => {
    const previousRollupData: [PreviousRollupData, PreviousRollupData] = [
      makePreviousRollupData(0x100),
      makePreviousRollupData(0x200),
    ];

    const historic_private_data_tree_sibling_path = range(PRIVATE_DATA_TREE_HEIGHT, 0x3000).map(x => fr(x));
    const historic_contract_data_tree_sibling_path = range(PRIVATE_DATA_TREE_HEIGHT, 0x4000).map(x => fr(x));

    const rootRollupInputs = RootRollupInputs.from({
      previousRollupData,
      newHistoricPrivateDataTreeRootSiblingPath: historic_private_data_tree_sibling_path,
      newHistoricContractDataTreeRootSiblingPath: historic_contract_data_tree_sibling_path,
    });

    const wasm = await CircuitsWasm.new();
    await expectSerializeToMatchSnapshot(
      rootRollupInputs.toBuffer(),
      'abis__test_roundtrip_serialize_root_rollup_inputs',
      wasm,
    );
  });

  it(`serializes and prints RootRollupPublicInputs`, async () => {
    const rootRollupPublicInputs = makeRootRollupPublicInputs();

    await expectSerializeToMatchSnapshot(
      rootRollupPublicInputs.toBuffer(),
      'abis__test_roundtrip_serialize_root_rollup_public_inputs',
    );
  });

  it(`serializes and deserializes RootRollupPublicInputs`, async () => {
    const rootRollupPublicInputs = makeRootRollupPublicInputs();

    await expectReserializeToMatchObject(
      rootRollupPublicInputs,
      'abis__test_roundtrip_reserialize_root_rollup_public_inputs',
      RootRollupPublicInputs.fromBuffer,
    );
  });
});
