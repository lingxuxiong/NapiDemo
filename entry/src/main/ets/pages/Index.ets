import { hilog } from '@kit.PerformanceAnalysisKit';

import DemoSDK from 'libentry.so';
import { ImageUtils } from '../utils/ImageUtils';

let LOG_TAG = "NAPIDemo"
let SAMPLE_FILE_NAME = "sample.jpg"

@Entry
@Component
struct Index {
  @State message: string = 'Hello NAPI';

  build() {
    Row() {
      Column() {
        Text(this.message)
          .fontSize(50)
          .fontWeight(FontWeight.Bold)
          .onClick(() => {
            hilog.info(0x0000, LOG_TAG, 'Test NAPI 2 + 3 = %{public}d', DemoSDK.add(2, 3));
          })

        Image($rawfile("sample.jpg"))
          .alignSelf(ItemAlign.Center)
          .width(400)
          .height(400)

        Button("Print Image")
          .onClick(() => {

            DemoSDK.callWithPromise(10, (doubledNum: number) => {
              return new Promise<string>((resolve, reject) => {
                resolve(doubledNum.toString());
              });
            })

            // copy the sample raw file to external storage to simulate the file to print
            // ImageUtils.copyRawFile(getContext(this), SAMPLE_FILE_NAME);
            //
            // let jobId = 1;
            //
            // DemoSDK.print(jobId,
            //   (pageNum: number) => {
            //     return new Promise<string>((resolve, reject) => {
            //       let filePath = getContext(this).filesDir + "/" + SAMPLE_FILE_NAME;
            //       hilog.info(0x0000, LOG_TAG, 'prepared file path for page %{public}d: %{public}s', pageNum, filePath);
            //       resolve(filePath);
            //     });
            //   },
            //
            //   (filePath: string) => {
            //     hilog.info(0x0000, LOG_TAG, 'ask file data for path: %{public}s', filePath);
            //     return new Promise<ArrayBuffer>((resolve, reject) => {
            //       // return image data on filePath
            //       ImageUtils.pixelMapOfFile(filePath).then( pm => {
            //         let len = pm.getPixelBytesNumber()
            //         let data = new ArrayBuffer(len)
            //         pm.readPixelsToBuffer(data).then( () => {
            //           console.log('Succeeded in reading image pixel data.');
            //           resolve(data);
            //         }).catch((error : Error) => {
            //           reject(error);
            //           console.error('failed to read image pixel data due to error: ' + error.message);
            //         });
            //       }).catch((error : Error) => {
            //         reject(error);
            //         console.error('failed to read pixel map due to error: ' + error.message);
            //       });
            //     });

                // return fake data
                // return ImageUtils.fakeDataArray();
            // });
          })
      }
      .width('100%')
    }
    .height('100%')
  }

}
