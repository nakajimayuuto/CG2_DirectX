# 注意
本プロジェクトは全画面で実行されます。
プロジェクトはESCキー、またはImGuiのFinishにて終了できます。

# 操作方法
## Camera
WASDまたはRスティックにて、移動マウスまたはLスティックにて回転、<br>
SPACEまたはBボタンで上昇、LSHIFTまたはAボタンで下降ができます。<br>
<br>

## ImGui
Resetでシーンの初期化ができ、。<br>
Finishでプログラムの終了ができます。<br>

### Models
ModelsタブではModelTypeにてモデルを指定し、Createボタンでモデルを生成することができます。<br>
また、それぞれのModel毎にタブがあり、TransformやLighting、UVTransform、モデルの削除等を変更することができます。<br>

### Light
LightタブではDirectionalLightの色、方向、輝度を変更することができます。<br>

### Sound
Soundタブでは音声のループ再生、停止、効果音としての再生(重複でき、停止の影響を受けない音声)が出来ます。

### FakeWindow
FakeWindowタブではuseFakeWindowをボタンでTrueにすることで疑似ウィンドウモードにすることができます。<br>
疑似ウィンドウモードでFakeWindowタブのCreateボタンを押すと疑似ウィンドウを増やすことができます。<br>
また、それぞれのWindowタブを開くとTransformの変更とWindowの削除ができます。

# 実装した加点要素
## 球の描画
CG2_05_00にて作成した方法で描画。
## Rambertian Reflectance
ModelsタブのLightingTypeをkLambertに変更することで確認可能。<br>
DirectionalLightはLightタブにて変更可能。
## Half Rambert
ModelsタブのLightingTypeをkHalfLambertに変更することで確認可能。<br>
DirectionalLightはLightタブにて変更可能。
## UVTransform
ModelsタブのuvScale、uvRotate、uvTranslateにて確認可能。
## 複数モデルの描画
実行時点で複数のモデルが描画されています。<br>
またModelsタブのModelTypeからモデルの見た目を指定して<br>
CreateModelボタンを押すことでモデルを追加することもできます。
## Utah Teapotの描画
実行時点で描画されています。<br>
UtahTeapotを選択した状態でCreateModelボタンを押すことで追加することもできます。
## Sound
CG2_07_00を基に作成し、クラス化とMicrosoft Media Fondationへの適応を行いました。<br>
SoundタブのPlayでループ再生、Stopで再生の停止、seで効果音として再生(ループ再生されず、重複して鳴らすことができる)が出来ます。
## GamePad
CG2_07_01をもとにXInputを利用して実装しました。<br>
本プロジェクトではカメラの移動にてGamePadを使用できます。
## Stanford Bunnyの描画
実行時点で描画されています。<br>
StanfordBunnyを選択した状態でCreateModelボタンを押すことで追加することもできます。
## MutliMesh対応
本プロジェクトではMultiMeshは同じトランスフォームで動く複数のオブジェクトとして実装しています。<br>
ModelsタブのMultiMeshタブにてメッシュごとにUVやLightingを変更可能です。
## MutliMaterial対応
MultiMeshと同じく、同じトランスフォームで動く複数のオブジェクトとして実装しています。<br>
ModelsタブのMultiMeshタブにてメッシュごとにUVやLightingを変更可能です。
## Suzanneの描画
実行時点で描画されています。<br>
Textureデータの存在しないモデルを読み込んだ場合真っ白なTextureを適応して描画するように実装しています。<br>
## Lighting方式の変更
ModelsタブのそれぞれのモデルのLightingTypeを変更することで確認可能です。
## ドキュメント
本ドキュメントにて成果物の情報を説明しています。
## その他
FakeWindowタブのuseFakeWindowをTrueにすると疑似ウィンドウを確認することができます。<br>
FakeWindowはStencilBufferと背景透過を利用して作成しています。<br>
CreateWindowにて疑似ウィンドウを作成できます。<br>
FakeWindowタブ内のそれぞれのWindowタブにて疑似ウィンドウのTransformの変更と、疑似ウィンドウの削除ができます。<br>