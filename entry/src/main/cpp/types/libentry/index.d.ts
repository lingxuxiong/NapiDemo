export const producePromise: (num: number) => Promise<number>;

export const consumePromise: (
  num: number,
  callback: (doubledNum: number) => Promise<string>
) => void;

export const print: (
  id: number,
  filePathCallback: (pageNum: number) => Promise<String>,
  fileDataCallback: (filePath: string) => Promise<ArrayBuffer>)
=> void;

export class JSBind {
  static bindFunction(name: string, func: Function): number;
}