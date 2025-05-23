import { MainPage } from 'libnativerender';

@Entry
@Component
struct Index {
  @State message: string = 'Hello World';

  build() {
    Column() {
      MainPage({
        fontFilepath: '/data/storage/el2/base/haps/entry/files/tex/res//fonts/base/cmmi10.ttf'
      })
        .height('48%')
        .width('100%');

      MainPage({
        fontFilepath: '/data/storage/el2/base/haps/entry/files/tex/res//fonts/base/cmex10.ttf'
      })
        .height('48%')
        .width('100%');
    }
    .height('100%')
    .width('100%')
    .backgroundColor('#ffe6e7e6');
  }
}