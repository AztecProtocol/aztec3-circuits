import { serializeToBuffer } from "../wasm/serialize.js";
import { ComposerType } from "./shared.js";
import { VerificationReferenceString } from "./verifier_reference_string.js";

/**
 * Kate commitment key object for verifying pairing equations.
 * @see honk/pcs/commitment_key.hpp.
 */
export class VerificationKey {
  constructor(
    /**
     * The number of gates in this circuit.
     */
    public numGates: number,
    /**
     * The number of inputs in this circuit.
     */
    public numInputs: number,
    /**
     * The system reference string for non-interactive arguments.
     */
    public verificationReferenceString: VerificationReferenceString,
    /**
     * Composer prover type we're using.
     */
    public composerType: ComposerType
  ) {}

  /**
   * Serialize as a buffer.
   * @returns The buffer.
   */
  toBuffer() {
    return serializeToBuffer(
      this.numGates,
      this.numInputs,
      this.verificationReferenceString,
      this.composerType
    );
  }
}
