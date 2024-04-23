import fs from '@ohos.file.fs';
import image from '@ohos.multimedia.image';

export class ImageUtils {

  static async scale(imgFileUri: string): Promise<image.PixelMap> {
    return ImageUtils.pixelMapOfFile(imgFileUri).then(pixelMap => {
      pixelMap.getImageInfo().then(info => {
        console.log("image width: ", info.size.width);
        console.log("image height: ", info.size.height);
        pixelMap.scale(0.5, 0.5);
      }).catch((err: Error) => {
        console.log("failed to get image info: ", err.message);
      });
      return pixelMap;
    });
  }

  static async pixelMapOfFile(filePath: string): Promise<image.PixelMap> {
    const imageSource = image.createImageSource(filePath);
    const pixelMap = await imageSource.createPixelMap({
      editable: true,
      desiredPixelFormat: 3,
    })
    return pixelMap;
  }

  static async pixelMapOfRawImageFile(context: Context, fileName: string): Promise<image.PixelMap> {
    const rm = context.resourceManager;
    const fileData = await rm.getRawFileContent(fileName);
    const buffer = fileData.buffer;
    const imageSource = image.createImageSource(buffer.slice(0, buffer.byteLength))
    const pixelMap = await imageSource.createPixelMap({
      editable: true,
      desiredPixelFormat: 3,
    })
    return pixelMap;
  }

  static async readImage(filePath: string): Promise<void> {
    ImageUtils.pixelMapOfFile(filePath).then(pm => {
      console.log("PixelBytesNumber:", pm.getPixelBytesNumber());
      console.log("BytesNumberPerRow:", pm.getBytesNumberPerRow());
      console.log("Density:", pm.getDensity());

      const area: image.PositionArea = {
        pixels: new ArrayBuffer(8),
        offset: 0,
        stride: 8,
        region: {
          x: 0,
          y: 0,
          size: {
            width: 2,
            height: 1
          }
        },
      };

      pm.readPixels(area);

      const writeColor = new ArrayBuffer(96);
      pm.writeBufferToPixels(writeColor);
    });
  }

  static async readEXIF(filePath: string): Promise<void> {
    // create image source
    const imageSource = image.createImageSource(filePath);

    // read exif info
    imageSource.getImageProperty(image.PropertyKey.BITS_PER_SAMPLE).then( bitsPerSample => {
      console.info("exif bitsPerSample of image ", filePath, " is ", bitsPerSample);
    }).catch((err: Error) => {
      console.error("failed to get exif bitsPerSample of image ", filePath, " due to: ", err.message);
    });

    // write exif info
    imageSource.modifyImageProperty(image.PropertyKey.IMAGE_WIDTH, '120').then(() => {
      imageSource.getImageProperty(image.PropertyKey.IMAGE_WIDTH).then( width => {
        console.info('exif of the new imageWidth is ', width);
      });
    })
  }

  static fakeDataArray(): ArrayBuffer {
    let buf = new ArrayBuffer(8);
    let res = new Uint8Array(buf);
    res[0] = 2;
    res[1] = 0;
    res[2] = 2;
    res[3] = 4;
    res[4] = 0;
    res[5] = 4;
    res[6] = 2;
    res[7] = 3;
    return res;
  }

  static copyRawFile(context: Context, from: string, to: string = from) {
    const resourceMgr = context.resourceManager;
    const srcFileData = resourceMgr.getRawFileContentSync(from);
    let dstFilePath = context.filesDir + "/" + to;
    let dstFile = fs.openSync(dstFilePath, fs.OpenMode.READ_WRITE | fs.OpenMode.CREATE);
    console.log('successfully saved raw file ', from, ' to ', dstFilePath);
    fs.writeSync(dstFile.fd, srcFileData.buffer.slice(0, srcFileData.buffer.byteLength));
    fs.closeSync(dstFile);
  }

}