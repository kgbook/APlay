plugins {
    id("com.android.library")
}

android {
    namespace = "com.kgbook.aplay.sdk"
    compileSdk = 35
    ndkVersion = "25.1.8937393"

    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_1_8
        targetCompatibility = JavaVersion.VERSION_1_8
    }

    defaultConfig {
        minSdk = 26

        ndk {
            abiFilters += listOf("armeabi-v7a", "arm64-v8a")
        }

        consumerProguardFiles("consumer-rules.pro")

        externalNativeBuild {
            cmake {
                arguments += listOf(
                    "-DAPLAY_BUILD_LINUX_APP=OFF",
                    "-DAPLAY_BUILD_HARNESS=OFF",
                    "-DAPLAY_BUILD_JNI_BINDING=ON",
                    "-DAPLAY_BUILD_NAPI_BINDING=OFF"
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
