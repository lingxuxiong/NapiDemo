export const add: (a: number, b: number) => number;

export const callWithPromise: (
  num: number,
  callback: (doubledNum: number) => Promise<string>
) => void;

export const print: (
  id: number,
  filePathCallback: (pageNum: number) => Promise<String>,
  fileDataCallback: (filePath: string) => Promise<ArrayBuffer>)
=> void;

export const asyncWork: (num: number) => Promise<number>;
