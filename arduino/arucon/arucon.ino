#include <TimerOne.h>

/**
 * デバッグ出力の切り替え
 */
#define DEBUG_MODE 0

/**
 * 最大入力受付数
 */
#define MAX_STEP 300

/**
 * 1入力受付毎にPCに返すデータ
 */
#define SIGNAL "SIGNAL"

/**
 * ボタンの出力ピン
 */
#define CIRCLE_PIN   9
#define TRIANGLE_PIN 8
#define CROSS_PIN    7
#define SQUARE_PIN   6

#define RIGHT_PIN    5
#define DOWN_PIN     4
#define UP_PIN       3
#define LEFT_PIN     2

#define L1_PIN       10
#define L2_PIN       11
#define R1_PIN       12
#define R2_PIN       17

#define START_PIN    14
#define PS_PIN       15
#define SELECT_PIN   16

/**
 * ボタンのビット変換
 */
#define CIRCLE   1
#define TRIANGLE 2
#define CROSS    4
#define SQUARE   8

#define RIGHT    16
#define DOWN     32
#define UP       64
#define LEFT     128

#define L1       1
#define L2       2
#define R1       4
#define R2       8

#define START    16
#define PS       32
#define SELECT   64

/**
 * 操作1つを表す構造体
 * frame      入力時間（1/60秒）
 * mainButton 十字キーと○×△□の4ボタン
 * subButton  その他のボタン
 * last       最後の操作かどうかを表す
 */
struct Step {
  short frame;
  byte mainButton;
  byte subButton;
  byte last;
};

volatile int commandIndex = 0;
volatile int frameCount = 0;
volatile boolean isRunning = false;
struct Step command[MAX_STEP];

void setup() {
  // 各ピンのモードを設定
  pinMode(START_PIN,    OUTPUT);
  pinMode(SELECT_PIN,   OUTPUT);
  pinMode(PS_PIN,       OUTPUT);
  pinMode(CIRCLE_PIN,   OUTPUT);
  pinMode(TRIANGLE_PIN, OUTPUT);
  pinMode(CROSS_PIN,    OUTPUT);
  pinMode(SQUARE_PIN,   OUTPUT);
  pinMode(RIGHT_PIN,    OUTPUT);
  pinMode(DOWN_PIN,     OUTPUT);
  pinMode(UP_PIN,       OUTPUT);
  pinMode(LEFT_PIN,     OUTPUT);
  pinMode(L1_PIN,       OUTPUT);
  pinMode(L2_PIN,       OUTPUT);
  pinMode(R1_PIN,       OUTPUT);
  pinMode(R2_PIN,       OUTPUT);

  Serial.begin(9600);

  // タイマ設定 16667us = 16.667ms = 1F
  Timer1.initialize();
  Timer1.attachInterrupt(frameExec, 16667);
}

/**
 * コマンドの読み込みをループさせる
 */
void loop() {
  if (Serial.available() > 0 && isRunning == false) {
    // コマンドの読み込み
    readCommand();

    // デバッグ出力
    if (DEBUG_MODE) {
      printStep();
    }
  }

  delay(50);
}

/**
 * コマンドの読み込みを行う関数
 */
void readCommand() {
  int index = 0;

  // コマンドをリセットする
  resetCommand();

  while (true) {
    // 操作を1つ読み込む
    readStep(index);

    // 操作の読み込みが全て終了したらコマンドを実行
    if (command[index].last == true) {
      isRunning = true;
      return;
    } else {
      index++;
    }
  }
}

/**
 * コマンドの初期化
 */
void resetCommand() {
  for (int i = 0; i < MAX_STEP; i++) {
    isRunning = false;
    command[i].frame = 0;
    command[i].mainButton = 0;
    command[i].subButton = 0;
    command[i].last = false;
  }
}

/**
 * 操作を1つ読み込む関数
 */
void readStep(int index) {
  // 何フレームの操作か読み込む
  command[index] = readFrame(command[index]);
  // 最後の操作であればボタンの情報を読み込まず終了
  if (command[index].last == true) {
    return;
  }

  // ボタンの情報を読み込む
  command[index] = readButtons(command[index]);
}

/**
 * 操作フレーム数を読み込む関数
 * Stepを受け取り受け取ったstepのframeに読み込んだフレームを入れて返す
 */
struct Step readFrame(struct Step step) {
  char buf[5] = {0};

  // コンマまで文字列を読み込みintにしてframeに入れる
  if (readStr(buf, ',') == -1) {
    step.last = true;
  }
  step.frame = atoi(buf);

  return step;
}

/**
 * ボタン入力を読み込む関数
 * Stepを受け取り受け取ったstepのmainButtonとsubButtonに読み込んだデータを入れて返す
 */
struct Step readButtons(struct Step step) {
  char buf[20] = {0};
  step.mainButton = 0;
  step.subButton = 0;

  readStr(buf, '\n');
  Serial.println(SIGNAL);

  // byteを1から2倍しながら有効であればmainButtonに足していく
  byte b = 1;
  for (int i = 0; i < 8; i++) {
    if (buf[i] == '1') {
      step.mainButton += b;
    }
    b *= 2;
  }

  // byteを1から2倍しながら有効であればsubButtonに足していく
  b = 1;
  for (int i = 8; i < 15; i++) {
    if (buf[i] == '1') {
      step.subButton += b;
    }
    b *= 2;
  }

  return step;
}

/**
 * 文字列を読み込む関数
 * シリアル通信のデータをdelimiterと一致するまで読み込み、受け取ったchar型の配列に入れて1を返す
 * delimiterを読み込んで終了したときは0, 終端文字を読み込んだときは-1を返す
 */
int readStr(char *str, char delimiter) {
  int i = 0;
  byte c;

  while (true) {
    if (Serial.available() > 0) {
      c = Serial.read();
      if (c == delimiter){
        // delimiterを読み込んだ場合
        return 0;
      } else if (c == '\0') {
        // 終端文字を読み込んだ場合
        return -1;
      } else {
        // 次の文字を読み込む
        str[i] = c;
        i++;
      }
    }
  }
  return 1;
}

/**
 * 1Fおきに実行する関数
 * isRunningがtrueの時に入力されているコマンドを実行する
 */
void frameExec() {
  interrupts();
  
  if (isRunning == true) {
    // 最後の操作かつ一つ前のStepが終了していれば全ての状態をリセットしてコマンド終了
    if (command[commandIndex].last == true && command[commandIndex - 1].frame <= frameCount) {
      resetButton();
      resetCommand();
      isRunning = false;
      commandIndex = 0;
      return;
    }

    /**
     * コマンドの先頭であれば強制的に実行
     * それ以外の場合は1つ前のStepが終了していれば次のボタン操作に移行
     */
    if (commandIndex == 0) {
      // 最初の操作の場合強制的に実行
      pushButton(&command[0]);
      commandIndex = 1;
      frameCount = 1;
    } else {
      // frameCountが現在の操作を越えたら次の操作を実行してframeCountを1に
      if (command[commandIndex - 1].frame <= frameCount) {
        pushButton(&command[commandIndex]);
        commandIndex++;
        frameCount = 1;
      } else {
        // frameCountを1増やす
        frameCount++;
      }
    }
  }
}

/**
 * Stepのポインタを受け取りボタンの入力状態を切り替える
 */
void pushButton(struct Step *step) {
  // mainButton
  digitalWrite(CIRCLE_PIN, step->mainButton & CIRCLE);
  digitalWrite(TRIANGLE_PIN, step->mainButton & TRIANGLE);
  digitalWrite(CROSS_PIN, step->mainButton & CROSS);
  digitalWrite(SQUARE_PIN, step->mainButton & SQUARE);
  digitalWrite(RIGHT_PIN, step->mainButton & RIGHT);
  digitalWrite(DOWN_PIN, step->mainButton & DOWN);
  digitalWrite(UP_PIN, step->mainButton & UP);
  digitalWrite(LEFT_PIN, step->mainButton & LEFT);

  // subButton
  digitalWrite(L1_PIN, step->subButton & L1);
  digitalWrite(L2_PIN, step->subButton & L2);
  digitalWrite(R1_PIN, step->subButton & R1);
  digitalWrite(R2_PIN, step->subButton & R2);
  digitalWrite(START_PIN, step->subButton & START);
  digitalWrite(SELECT_PIN, step->subButton & SELECT);
  digitalWrite(PS_PIN, step->subButton & PS);
}

/**
 * 全てのボタンを離した状態に切り替える
 */
void resetButton() {
  digitalWrite(CIRCLE_PIN, LOW);
  digitalWrite(TRIANGLE_PIN, LOW);
  digitalWrite(CROSS_PIN, LOW);
  digitalWrite(SQUARE_PIN, LOW);
  digitalWrite(RIGHT_PIN, LOW);
  digitalWrite(DOWN_PIN, LOW);
  digitalWrite(UP_PIN, LOW);
  digitalWrite(LEFT_PIN, LOW);
  digitalWrite(L1_PIN, LOW);
  digitalWrite(L2_PIN, LOW);
  digitalWrite(R1_PIN, LOW);
  digitalWrite(R2_PIN, LOW);
  digitalWrite(START_PIN, LOW);
  digitalWrite(SELECT_PIN, LOW);
  digitalWrite(PS_PIN, LOW);
}

/**
 * デバッグ表示用関数
 */
void printStep() {
  int index = 0;

  Serial.println("######################");
  while (true) {
    Serial.print("Index: ");
    Serial.println(index);
    Serial.print("  frame: ");
    Serial.println(command[index].frame);
    Serial.print("  last: ");
    Serial.println(command[index].last);
    Serial.print("  button: ");
    Serial.println(command[index].mainButton);

    if (command[index].last == true){
      Serial.println("######################");
      return;
    }
    index++;
  }
}
