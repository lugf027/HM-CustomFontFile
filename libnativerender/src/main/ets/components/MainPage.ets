import { hilog } from '@kit.PerformanceAnalysisKit';
import testNapi from 'liblibnativerender.so';
import nativeRender from 'liblibnativerender.so';

const DOMAIN = 0x0000;
const TAG = 'TextTestMainPage';

class CustomXComponentController extends XComponentController {
  onSurfaceCreated(surfaceId: string): void {
    hilog.info(0x0000, TAG, '%{public}s', `CustomXComponentController onSurfaceCreated surfaceId: ${surfaceId}`);
    nativeRender.SetSurfaceId(BigInt(surfaceId));
  }

  onSurfaceChanged(surfaceId: string, rect: SurfaceRect): void {
    hilog.info(0x0000, TAG, '%{public}s',
      `CustomXComponentController onSurfaceChanged surfaceId: ${surfaceId}, rect: ${JSON.stringify(rect)}}`);
    nativeRender.ChangeSurface(BigInt(surfaceId), rect.surfaceWidth, rect.surfaceHeight);
  }

  onSurfaceDestroyed(surfaceId: string): void {
    hilog.info(0x0000, TAG, '%{public}s', `CustomXComponentController onSurfaceDestroyed surfaceId: ${surfaceId}`);
    nativeRender.DestroySurface(BigInt(surfaceId));
  }
}

@Component
export struct MainPage {
  @Prop textSize: number = 40;
  @Prop fontFilepath: string = '';
  private xComponentController: XComponentController = new CustomXComponentController();

  build() {
    Row() {
      Column() {
        Text(`字体: ${this.fontFilepath}`)
          .fontSize(17)
          .fontWeight(FontWeight.Bold);

        Row() {
          Button('FontHeight 1')
            .fontSize('16fp')
            .fontWeight(500)
            .margin({ bottom: 24 })
            .onClick(() => {
              let surfaceId = this.xComponentController.getXComponentSurfaceId();
              this.xComponentController.getXComponentSurfaceRect();
              nativeRender.DrawWithCustomFont(BigInt(surfaceId), this.fontFilepath,
                `abcdefghigklmn opqrstuvwxyz 1234567890`, 1);
            })
            .margin(2);

          Button('FontHeight 0.01')
            .fontSize('16fp')
            .fontWeight(500)
            .margin({ bottom: 24 })
            .onClick(() => {
              let surfaceId = this.xComponentController.getXComponentSurfaceId();
              this.xComponentController.getXComponentSurfaceRect();
              nativeRender.DrawWithCustomFont(BigInt(surfaceId), this.fontFilepath,
                `abcdefghigklmn opqrstuvwxyz 1234567890`, 0.01);
            })
            .margin(2);

        }.width('100%');


        // native draw
        XComponent({
          type: XComponentType.SURFACE,
          controller: this.xComponentController,
        })
          .width('100%')
          .height('70%')
          .enableAnalyzer(true)
          .onClick(() => {
            let surfaceId = this.xComponentController.getXComponentSurfaceId();
            hilog.info(0x0000, TAG, '%{public}s', `Click xComponentController surfaceId:${surfaceId}`);
          });
      }
      .width('100%')
      .height('100%');
    }
    .width('100%')
    .height('100%');
  }
}