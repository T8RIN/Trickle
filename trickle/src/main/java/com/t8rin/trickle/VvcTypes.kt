package com.t8rin.trickle

enum class VvcContainer(internal val value: Int) {
    RAW_VVC(0),
    HEIF(1),
}

enum class VvcChroma(internal val value: Int) {
    MONOCHROME(0),
    YUV_420(1),
    YUV_422(2),
    YUV_444(3),
}

enum class VvcBitDepth(internal val value: Int) {
    EIGHT(8),
    TEN(10),
    TWELVE(12),
}

enum class VvcOrientation(internal val exifValue: Int) {
    NORMAL(1),
    FLIP_H(2),
    ROTATE_180(3),
    FLIP_V(4),
    TRANSPOSE(5),
    ROTATE_90(6),
    TRANSVERSE(7),
    ROTATE_270(8),
}

data class VvcCicp(
    val primaries: Primaries = Primaries.BT_709,
    val transfer: Transfer = Transfer.SRGB,
    val matrix: Matrix = Matrix.SMPTE_170M,
    val fullRange: Boolean = true,
) {
    enum class Primaries(internal val value: Int) {
        BT_709(1),
        UNSPECIFIED(2),
        BT_470_M(4),
        BT_470_BG(5),
        BT_601(6),
        SMPTE_240(7),
        GENERIC_FILM(8),
        BT_2020(9),
        XYZ(10),
        SMPTE_431(11),
        SMPTE_432(12),
        EBU_3213(22),
    }

    enum class Transfer(internal val value: Int) {
        BT_709(1),
        UNSPECIFIED(2),
        BT_470_M(4),
        BT_470_BG(5),
        BT_601(6),
        SMPTE_240(7),
        LINEAR(8),
        LOG_100(9),
        LOG_100_SQRT_10(10),
        IEC_61966(11),
        BT_1361(12),
        SRGB(13),
        BT_2020_10_BIT(14),
        BT_2020_12_BIT(15),
        SMPTE_2084(16),
        SMPTE_428(17),
        HLG(18),
    }

    enum class Matrix(internal val value: Int) {
        IDENTITY(0),
        BT_709(1),
        UNSPECIFIED(2),
        FCC(4),
        BT_470_BG(5),
        SMPTE_170M(6),
        SMPTE_240M(7),
        YCGCO(8),
        BT_2020_NCL(9),
        BT_2020_CL(10),
        SMPTE_2085(11),
        CHROMATICITY_DERIVED_NCL(12),
        CHROMATICITY_DERIVED_CL(13),
        ICTCP(14),
        IPT_C2(15),
        YCGCO_RE(16),
        YCGCO_RO(17),
    }

    companion object {
        val DEFAULT = VvcCicp()
        val SRGB = VvcCicp(matrix = Matrix.BT_709)
        val BT_709 = VvcCicp(transfer = Transfer.BT_709, matrix = Matrix.BT_709)
        val BT_2020_PQ = VvcCicp(
            primaries = Primaries.BT_2020,
            transfer = Transfer.SMPTE_2084,
            matrix = Matrix.BT_2020_NCL
        )
    }
}

data class VvcContentLightLevel(
    val maxContentLightLevel: Int,
    val maxPictureAverageLightLevel: Int,
)
