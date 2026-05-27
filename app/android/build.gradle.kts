plugins {
    id("com.android.application") version "8.7.3"
}

android {
    namespace = "com.kgbook.aplay"
    compileSdk = 35
    ndkVersion = "25.1.8937393"

    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_1_8
        targetCompatibility = JavaVersion.VERSION_1_8
    }

    defaultConfig {
        applicationId = "com.kgbook.aplay"
        minSdk = 26
        targetSdk = 35
        versionCode = 1
        versionName = "0.1.0"

        externalNativeBuild {
            cmake {
                arguments += listOf(
                    "-DAPLAY_BUILD_LINUX_APP=OFF",
                    "-DAPLAY_BUILD_HARNESS=OFF",
                    "-DAPLAY_BUILD_ANDROID_NATIVE=ON"
                )
            }
        }
    }

    externalNativeBuild {
        cmake {
            path = file("src/main/cpp/CMakeLists.txt")
        }
    }
}
