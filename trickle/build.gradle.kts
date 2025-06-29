plugins {
    id("org.jetbrains.kotlin.android")
    id("com.android.library")
    id("maven-publish")
}

afterEvaluate {
    publishing {
        publications {
            create<MavenPublication>("mavenJava") {
                groupId = "com.github.t8rin"
                artifactId = "trickle"
                version = libs.versions.libVersion.get()
                from(components["release"])
            }
        }
    }
}

android {
    namespace = "com.t8rin.trickle"
    defaultConfig {
        ndk.abiFilters.addAll(listOf("armeabi-v7a", "arm64-v8a", "x86", "x86_64"))
    }
    externalNativeBuild {
        cmake {
            path("src/main/cpp/CMakeLists.txt")
            ndkVersion = "26.1.10909125"
        }
    }
    sourceSets.named("main") {
        jniLibs {
            srcDir("src/main/libs")
        }
    }

    compileSdk = libs.versions.androidCompileSdk.get().toIntOrNull()

    defaultConfig {
        minSdk = libs.versions.androidMinSdk.get().toIntOrNull()
    }

    compileOptions {
        sourceCompatibility = javaVersion
        targetCompatibility = javaVersion
        isCoreLibraryDesugaringEnabled = true
    }

    dependencies {
        implementation(libs.androidxCore)
        coreLibraryDesugaring(libs.desugaring)
    }

    kotlinOptions {
        jvmTarget = javaVersion.toString()
    }
}

private val Project.javaVersion: JavaVersion
    get() = JavaVersion.toVersion(libs.versions.jvmTarget.get())