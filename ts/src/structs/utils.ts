export function checkLength<T>(arr: T[], expected: number, label: string) {
  if (arr.length !== expected) {
    throw new Error(
      `Invalid length for ${label} (got ${arr.length}, expected ${expected})`
    );
  }
}
