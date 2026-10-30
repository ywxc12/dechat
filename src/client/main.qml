import QtQuick
import QtQuick.Controls
import QtQuick.Effects // 用于阴影和发光效果

// 启动窗口（无边框）
Window {
    id: splashWindow
    width: 600
    height: 400
    visible: true
    color: "transparent" // 背景透明，为了画出圆角和阴影
    flags: Qt.SplashScreen | Qt.FramelessWindowHint

    // 背景卡片
    Rectangle {
        id: bgCard
        anchors.fill: parent
        anchors.margins: 20
        color: "#2b2d30" // IDEA 经典的深灰色
        radius: 12
        
        // 核心阴影效果
        layer.enabled: true
        layer.effect: MultiEffect {
            shadowEnabled: true
            shadowBlur: 20
            shadowColor: "#80000000"
        }

        // 居中 Logo (用文字代替，你可以换成你的图片)
        Text {
            anchors.centerIn: parent
            text: "dechat"
            font.family: "Consolas"
            font.pixelSize: 48
            font.bold: true
            color: "#4a88c7" // 类似 IDEA 的蓝色
            
            // 呼吸灯缩放动画
            SequentialAnimation on scale {
                loops: Animation.Infinite
                NumberAnimation { to: 1.05; duration: 1000; easing.type: Easing.InOutQuad }
                NumberAnimation { to: 1.0; duration: 1000; easing.type: Easing.InOutQuad }
            }
        }

        // 底部 Loading 进度条[有待修改]
        Rectangle {
            width: parent.width - 80
            height: 4
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 40
            anchors.horizontalCenter: parent.horizontalCenter
            color: "#1e1f22" // 进度条底色
            radius: 2

            Rectangle {
                id: progressBar
                height: parent.height
                width: 0
                color: "#4a88c7" // 进度条亮色
                radius: 2

                // 模拟加载动画 (你可以根据 C++ 真实的初始化进度来绑定)
                NumberAnimation on width {
                    from: 0
                    to: parent.width
                    duration: 2500
                    easing.type: Easing.OutCubic
                    onFinished: {
                        // 加载完毕，关闭启动界面，显示主窗口
                        console.log("加载完成！切换到主界面...")
                        splashWindow.close()
                    }
                }
            }
        }
        
        // 底部小字版本号
        Text {
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 15
            anchors.horizontalCenter: parent.horizontalCenter
            text: "Version 0.1 (Build 2026)"
            color: "#6b6f76"
            font.pixelSize: 12
        }
    }
}