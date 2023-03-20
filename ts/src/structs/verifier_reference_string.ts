/**
 * The verifier reference string for our prover system.
 * Used for non-interactive argument proofs (i.e. SNARKs).
 */
export class VerificationReferenceString {
  constructor(
    /**
     * The reference string.
     */
    public data: Buffer
  ) {}

  /**
   * Serialize as a buffer.
   * TODO explore more efficient ways of writing this into shared_ptr.
   * @returns The buffer.
   */
  toBuffer() {
    return this.data;
  }
}
