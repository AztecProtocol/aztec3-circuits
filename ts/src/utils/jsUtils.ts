export function range(n: number) {
  const ret: number[] = [];
  for (let i = 0; i < n; i++) {
    ret.push(i);
  }
  return ret;
}

export function assertLength(obj: any, member: string, length: number) {
  if (obj[member].length !== length) {
    throw new Error(
      `Expected ${member} to have length ${length}! Was: ${obj[member].length}`
    );
  }
}
