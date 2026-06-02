import com.android.build.gradle.internal.api.BaseVariantOutputImpl

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
    }
}

android.applicationVariants.all {
    val variantName = name
    outputs.all {
        (this as BaseVariantOutputImpl).outputFileName = "APlayReceiver-${variantName}.apk"
    }
}

dependencies {
    implementation(project(":APlaySdk"))
}
