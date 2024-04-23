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

            DemoSDK.asyncWork(1024).then((result) => {
              hilog.info(0x0000, LOG_TAG, 'async result %{public}d', result);
            }).catch((error : Error) => {
              hilog.error(0x0000, LOG_TAG, 'failed to get async task due to error: ' + error.message);
            });

            // copy the sample raw file to external storage to simulate the file to print
            ImageUtils.copyRawFile(getContext(this), SAMPLE_FILE_NAME);

            let jobId = 1;

            DemoSDK.print(jobId,
              (pageNum: number) => {
                let filePath = getContext(this).filesDir + "/" + SAMPLE_FILE_NAME;
                hilog.info(0x0000, LOG_TAG, 'prepared file path for page %{public}d: %{public}s', pageNum, filePath);
                return filePath;
              },
              (filePath: string) => {
                hilog.info(0x0000, LOG_TAG, 'ask file data for path: %{public}s', filePath);

                // return image data on filePath
                ImageUtils.pixelMapOfFile(filePath).then( pm => {
                  let len = pm.getPixelBytesNumber()
                  let data = new ArrayBuffer(len)
                  pm.readPixelsToBuffer(data).then( () => {
                    hilog.error(0x0000, LOG_TAG, 'Succeeded in reading image pixel data.');
                  }).catch((error : Error) => {
                    hilog.error(0x0000, LOG_TAG, 'failed to read image pixel data due to error: ' + error.message);
                  });
                }).catch((error : Error) => {
                  hilog.error(0x0000, LOG_TAG, 'failed to read pixel map due to error: ' + error.message);
                });

                // return fake data
                return ImageUtils.fakeDataArray();
            });
          })
      }
      .width('100%')
    }
    .height('100%')
  }

}
