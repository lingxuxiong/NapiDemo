import { hilog } from '@kit.PerformanceAnalysisKit';

import DemoSDK from 'libentry.so';
import { ImageUtils } from '../utils/ImageUtils';

let LOG_TAG = "NAPIDemo"
let SAMPLE_FILE_NAME = "sample.jpg"

@Entry
@Component
struct Index {

  @State message: string = 'Hello NAPI';
  @State num: number = 0;

  build() {
    Row() {
      Column() {
        Image($rawfile("sample.jpg"))
          .alignSelf(ItemAlign.Center)
          .width(400)
          .height(400)

        Button("Produce Promise")
          .onClick(() => {
            DemoSDK.producePromise(1024).then((result) => {
              hilog.info(0x0000, LOG_TAG, 'got async result %{public}d', result);
            }).catch((error : Error) => {
              hilog.error(0x0000, LOG_TAG, 'failed to get async task due to error: ' + error.message);
            });
          })

        Button("Consume Promise")
          .onClick(() => {
            this.num++;
            DemoSDK.consumePromise(this.num, (doubledNum: number) => {
              return new Promise<string>((resolve, reject) => {
                hilog.info(0x0000, LOG_TAG, 'new num value = %{public}d', this.num);
                if (this.num % 2 == 0) {
                  resolve("resolved: " + doubledNum.toString());
                } else {
                  reject("rejected: " + doubledNum.toString());
                }
              });
            })
          })

        Button("Print Image")
          .onClick(() => {
            // copy the sample raw file to external storage to simulate the file to print
            ImageUtils.copyRawFile(getContext(this), SAMPLE_FILE_NAME);

            let jobId = 1;

            DemoSDK.print(jobId,
              (pageNum: number) => {
                return new Promise<string>((resolve, reject) => {
                  let filePath = getContext(this).filesDir + "/" + SAMPLE_FILE_NAME;
                  hilog.info(0x0000, LOG_TAG, 'prepared file path for page %{public}d: %{public}s', pageNum, filePath);
                  resolve(filePath);
                });
              },

              (filePath: string) => {
                hilog.info(0x0000, LOG_TAG, 'ask data for file path: %{public}s', filePath);
                return new Promise<ArrayBuffer>((resolve, reject) => {
                  // return image data on filePath
                  ImageUtils.pixelMapOfFile(filePath).then( pm => {
                    let len = pm.getPixelBytesNumber()
                    let data = new ArrayBuffer(len)
                    hilog.info(0x0000, LOG_TAG, 'file size: %{public}d bytes.', len);
                    pm.readPixelsToBuffer(data).then( () => {
                      hilog.info(0x0000, LOG_TAG, 'Succeeded in reading image pixel data.');
                      resolve(data);
                    }).catch((error : Error) => {
                      hilog.error(0x0000, LOG_TAG, 'failed to read image pixel data due to error: ' + error.message);
                      reject(error);
                    });
                  }).catch((error : Error) => {
                    hilog.error(0x0000, LOG_TAG, 'failed to read pixel map due to error: ' + error.message);
                    reject(error);
                  });
                });
            });
          })
      }
      .width('100%')
    }
    .height('100%')
  }

}
