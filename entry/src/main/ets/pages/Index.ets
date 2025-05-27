import { MainPage } from 'libnativerender';

@Entry
@Component
struct Index {
  @State message: string = 'Hello World';

  private fontList = [
    '/data/storage/el2/base/haps/entry/files/tex/res//fonts/base/cmmi10.ttf',
    '/data/storage/el2/base/haps/entry/files/tex/res//fonts/base/cmex10.ttf',
    '/data/storage/el2/base/haps/entry/files/tex/res//fonts/base/cmmib10.ttf',
    '/data/storage/el2/base/haps/entry/files/tex/res//fonts/latin/bi10.ttf',
    '/data/storage/el2/base/haps/entry/files/tex/res//fonts/latin/bx10.ttf',
    '/data/storage/el2/base/haps/entry/files/tex/res//fonts/latin/cmr10.ttf',
  ]

  build() {
    Column() {
      Scroll() {
        Column() {
          ForEach(this.fontList, (item: string) => {
            MainPage({ fontFilepath: item })
              .height('40%')
              .width('100%');
          });
        }
        .width('100%');
      }
      .layoutWeight(1)
      .width('100%');
    }
    .height('100%')
    .width('100%')
    .backgroundColor('#ffe6e7e6');
  }
}