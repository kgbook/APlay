import APlaySdk from '@kgbook/aplay-sdk';

@Entry
@Component
struct Index {
  build() {
    Column({ space: 16 }) {
      Text('APlay')
        .fontSize(32)
        .fontWeight(FontWeight.Bold)

      Text(`Harmony app using SDK ${APlaySdk.version()}`)
        .fontSize(18)
        .fontColor('#334155')
    }
    .width('100%')
    .height('100%')
    .justifyContent(FlexAlign.Center)
    .alignItems(HorizontalAlign.Center)
    .backgroundColor('#F8FAFC')
  }
}
