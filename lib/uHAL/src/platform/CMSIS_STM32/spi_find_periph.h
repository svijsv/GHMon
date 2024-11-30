//
// Generated by tools/cmsis/spi_find_periph.sh on Mon Nov 25 03:58:32 PM EST 2024
//

#if INCLUDED_BY_SPI_C


//
// SPI1
//
#if defined(SPI1)
# define HAVE_SPI1 1
#else
# define HAVE_SPI1 0
#endif

#if HAVE_SPI1
# if defined(PINID_SPI1_MOSI) && PINID_SPI1_MOSI > 0
#  define IS_SPI1_MOSI_DEF(_p_) (PINID(_p_) == PINID_SPI1_MOSI)
# else
#  define IS_SPI1_MOSI_DEF(_p_) (0)
# endif
# if defined(PINID_SPI1_MOSI_ALT2) && PINID_SPI1_MOSI_ALT2 > 0
#  define IS_SPI1_MOSI_ALT2(_p_) (PINID(_p_) == PINID_SPI1_MOSI_ALT2)
# else
#  define IS_SPI1_MOSI_ALT2(_p_) (0)
# endif
# if defined(PINID_SPI1_MOSI_ALT) && PINID_SPI1_MOSI_ALT > 0
#  define IS_SPI1_MOSI_ALT1(_p_) (PINID(_p_) == PINID_SPI1_MOSI_ALT)
# else
#  define IS_SPI1_MOSI_ALT1(_p_) (0)
# endif
# define IS_SPI1_MOSI(_p_) (IS_SPI1_MOSI_DEF(_p_) || IS_SPI1_MOSI_ALT1(_p_) || IS_SPI1_MOSI_ALT2(_p_))

# if defined(PINID_SPI1_MISO) && PINID_SPI1_MISO > 0
#  define IS_SPI1_MISO_DEF(_p_) (PINID(_p_) == PINID_SPI1_MISO)
# else
#  define IS_SPI1_MISO_DEF(_p_) (0)
# endif
# if defined(PINID_SPI1_MISO_ALT2) && PINID_SPI1_MISO_ALT2 > 0
#  define IS_SPI1_MISO_ALT2(_p_) (PINID(_p_) == PINID_SPI1_MISO_ALT2)
# else
#  define IS_SPI1_MISO_ALT2(_p_) (0)
# endif
# if defined(PINID_SPI1_MISO_ALT) && PINID_SPI1_MISO_ALT > 0
#  define IS_SPI1_MISO_ALT1(_p_) (PINID(_p_) == PINID_SPI1_MISO_ALT)
# else
#  define IS_SPI1_MISO_ALT1(_p_) (0)
# endif
# define IS_SPI1_MISO(_p_) (IS_SPI1_MISO_DEF(_p_) || IS_SPI1_MISO_ALT1(_p_) || IS_SPI1_MISO_ALT2(_p_))

# if defined(PINID_SPI1_SCK) && PINID_SPI1_SCK > 0
#  define IS_SPI1_SCK_DEF(_p_) (PINID(_p_) == PINID_SPI1_SCK)
# else
#  define IS_SPI1_SCK_DEF(_p_) (0)
# endif
# if defined(PINID_SPI1_SCK_ALT2) && PINID_SPI1_SCK_ALT2 > 0
#  define IS_SPI1_SCK_ALT2(_p_) (PINID(_p_) == PINID_SPI1_SCK_ALT2)
# else
#  define IS_SPI1_SCK_ALT2(_p_) (0)
# endif
# if defined(PINID_SPI1_SCK_ALT) && PINID_SPI1_SCK_ALT > 0
#  define IS_SPI1_SCK_ALT1(_p_) (PINID(_p_) == PINID_SPI1_SCK_ALT)
# else
#  define IS_SPI1_SCK_ALT1(_p_) (0)
# endif
# define IS_SPI1_SCK(_p_) (IS_SPI1_SCK_DEF(_p_) || IS_SPI1_SCK_ALT1(_p_) || IS_SPI1_SCK_ALT2(_p_))

# define IS_SPI1(_mosi_, _miso_, _sck_) (IS_SPI1_MOSI(_mosi_) && IS_SPI1_MISO(_miso_) && IS_SPI1_SCK(_sck_))

# if !defined(SPIx) && IS_SPI1(SPI_MOSI_PIN, SPI_MISO_PIN, SPI_SCK_PIN)
#  define SPIx SPI1
#  define SPIx_CLOCKEN RCC_PERIPH_SPI1
#  define SPIx_AF GPIOAF_SPI1
# endif

#else // HAVE_SPI1
# define IS_SPI1(_mosi_, _miso_, _sck_) (0)
#endif // HAVE_SPI1

//
// SPI2
//
#if defined(SPI2)
# define HAVE_SPI2 1
#else
# define HAVE_SPI2 0
#endif

#if HAVE_SPI2
# if defined(PINID_SPI2_MOSI) && PINID_SPI2_MOSI > 0
#  define IS_SPI2_MOSI_DEF(_p_) (PINID(_p_) == PINID_SPI2_MOSI)
# else
#  define IS_SPI2_MOSI_DEF(_p_) (0)
# endif
# if defined(PINID_SPI2_MOSI_ALT2) && PINID_SPI2_MOSI_ALT2 > 0
#  define IS_SPI2_MOSI_ALT2(_p_) (PINID(_p_) == PINID_SPI2_MOSI_ALT2)
# else
#  define IS_SPI2_MOSI_ALT2(_p_) (0)
# endif
# if defined(PINID_SPI2_MOSI_ALT) && PINID_SPI2_MOSI_ALT > 0
#  define IS_SPI2_MOSI_ALT1(_p_) (PINID(_p_) == PINID_SPI2_MOSI_ALT)
# else
#  define IS_SPI2_MOSI_ALT1(_p_) (0)
# endif
# define IS_SPI2_MOSI(_p_) (IS_SPI2_MOSI_DEF(_p_) || IS_SPI2_MOSI_ALT1(_p_) || IS_SPI2_MOSI_ALT2(_p_))

# if defined(PINID_SPI2_MISO) && PINID_SPI2_MISO > 0
#  define IS_SPI2_MISO_DEF(_p_) (PINID(_p_) == PINID_SPI2_MISO)
# else
#  define IS_SPI2_MISO_DEF(_p_) (0)
# endif
# if defined(PINID_SPI2_MISO_ALT2) && PINID_SPI2_MISO_ALT2 > 0
#  define IS_SPI2_MISO_ALT2(_p_) (PINID(_p_) == PINID_SPI2_MISO_ALT2)
# else
#  define IS_SPI2_MISO_ALT2(_p_) (0)
# endif
# if defined(PINID_SPI2_MISO_ALT) && PINID_SPI2_MISO_ALT > 0
#  define IS_SPI2_MISO_ALT1(_p_) (PINID(_p_) == PINID_SPI2_MISO_ALT)
# else
#  define IS_SPI2_MISO_ALT1(_p_) (0)
# endif
# define IS_SPI2_MISO(_p_) (IS_SPI2_MISO_DEF(_p_) || IS_SPI2_MISO_ALT1(_p_) || IS_SPI2_MISO_ALT2(_p_))

# if defined(PINID_SPI2_SCK) && PINID_SPI2_SCK > 0
#  define IS_SPI2_SCK_DEF(_p_) (PINID(_p_) == PINID_SPI2_SCK)
# else
#  define IS_SPI2_SCK_DEF(_p_) (0)
# endif
# if defined(PINID_SPI2_SCK_ALT2) && PINID_SPI2_SCK_ALT2 > 0
#  define IS_SPI2_SCK_ALT2(_p_) (PINID(_p_) == PINID_SPI2_SCK_ALT2)
# else
#  define IS_SPI2_SCK_ALT2(_p_) (0)
# endif
# if defined(PINID_SPI2_SCK_ALT) && PINID_SPI2_SCK_ALT > 0
#  define IS_SPI2_SCK_ALT1(_p_) (PINID(_p_) == PINID_SPI2_SCK_ALT)
# else
#  define IS_SPI2_SCK_ALT1(_p_) (0)
# endif
# define IS_SPI2_SCK(_p_) (IS_SPI2_SCK_DEF(_p_) || IS_SPI2_SCK_ALT1(_p_) || IS_SPI2_SCK_ALT2(_p_))

# define IS_SPI2(_mosi_, _miso_, _sck_) (IS_SPI2_MOSI(_mosi_) && IS_SPI2_MISO(_miso_) && IS_SPI2_SCK(_sck_))

# if !defined(SPIx) && IS_SPI2(SPI_MOSI_PIN, SPI_MISO_PIN, SPI_SCK_PIN)
#  define SPIx SPI2
#  define SPIx_CLOCKEN RCC_PERIPH_SPI2
#  define SPIx_AF GPIOAF_SPI2
# endif

#else // HAVE_SPI2
# define IS_SPI2(_mosi_, _miso_, _sck_) (0)
#endif // HAVE_SPI2

//
// SPI3
//
#if defined(SPI3)
# define HAVE_SPI3 1
#else
# define HAVE_SPI3 0
#endif

#if HAVE_SPI3
# if defined(PINID_SPI3_MOSI) && PINID_SPI3_MOSI > 0
#  define IS_SPI3_MOSI_DEF(_p_) (PINID(_p_) == PINID_SPI3_MOSI)
# else
#  define IS_SPI3_MOSI_DEF(_p_) (0)
# endif
# if defined(PINID_SPI3_MOSI_ALT2) && PINID_SPI3_MOSI_ALT2 > 0
#  define IS_SPI3_MOSI_ALT2(_p_) (PINID(_p_) == PINID_SPI3_MOSI_ALT2)
# else
#  define IS_SPI3_MOSI_ALT2(_p_) (0)
# endif
# if defined(PINID_SPI3_MOSI_ALT) && PINID_SPI3_MOSI_ALT > 0
#  define IS_SPI3_MOSI_ALT1(_p_) (PINID(_p_) == PINID_SPI3_MOSI_ALT)
# else
#  define IS_SPI3_MOSI_ALT1(_p_) (0)
# endif
# define IS_SPI3_MOSI(_p_) (IS_SPI3_MOSI_DEF(_p_) || IS_SPI3_MOSI_ALT1(_p_) || IS_SPI3_MOSI_ALT2(_p_))

# if defined(PINID_SPI3_MISO) && PINID_SPI3_MISO > 0
#  define IS_SPI3_MISO_DEF(_p_) (PINID(_p_) == PINID_SPI3_MISO)
# else
#  define IS_SPI3_MISO_DEF(_p_) (0)
# endif
# if defined(PINID_SPI3_MISO_ALT2) && PINID_SPI3_MISO_ALT2 > 0
#  define IS_SPI3_MISO_ALT2(_p_) (PINID(_p_) == PINID_SPI3_MISO_ALT2)
# else
#  define IS_SPI3_MISO_ALT2(_p_) (0)
# endif
# if defined(PINID_SPI3_MISO_ALT) && PINID_SPI3_MISO_ALT > 0
#  define IS_SPI3_MISO_ALT1(_p_) (PINID(_p_) == PINID_SPI3_MISO_ALT)
# else
#  define IS_SPI3_MISO_ALT1(_p_) (0)
# endif
# define IS_SPI3_MISO(_p_) (IS_SPI3_MISO_DEF(_p_) || IS_SPI3_MISO_ALT1(_p_) || IS_SPI3_MISO_ALT2(_p_))

# if defined(PINID_SPI3_SCK) && PINID_SPI3_SCK > 0
#  define IS_SPI3_SCK_DEF(_p_) (PINID(_p_) == PINID_SPI3_SCK)
# else
#  define IS_SPI3_SCK_DEF(_p_) (0)
# endif
# if defined(PINID_SPI3_SCK_ALT2) && PINID_SPI3_SCK_ALT2 > 0
#  define IS_SPI3_SCK_ALT2(_p_) (PINID(_p_) == PINID_SPI3_SCK_ALT2)
# else
#  define IS_SPI3_SCK_ALT2(_p_) (0)
# endif
# if defined(PINID_SPI3_SCK_ALT) && PINID_SPI3_SCK_ALT > 0
#  define IS_SPI3_SCK_ALT1(_p_) (PINID(_p_) == PINID_SPI3_SCK_ALT)
# else
#  define IS_SPI3_SCK_ALT1(_p_) (0)
# endif
# define IS_SPI3_SCK(_p_) (IS_SPI3_SCK_DEF(_p_) || IS_SPI3_SCK_ALT1(_p_) || IS_SPI3_SCK_ALT2(_p_))

# define IS_SPI3(_mosi_, _miso_, _sck_) (IS_SPI3_MOSI(_mosi_) && IS_SPI3_MISO(_miso_) && IS_SPI3_SCK(_sck_))

# if !defined(SPIx) && IS_SPI3(SPI_MOSI_PIN, SPI_MISO_PIN, SPI_SCK_PIN)
#  define SPIx SPI3
#  define SPIx_CLOCKEN RCC_PERIPH_SPI3
#  define SPIx_AF GPIOAF_SPI3
# endif

#else // HAVE_SPI3
# define IS_SPI3(_mosi_, _miso_, _sck_) (0)
#endif // HAVE_SPI3

//
// SPI4
//
#if defined(SPI4)
# define HAVE_SPI4 1
#else
# define HAVE_SPI4 0
#endif

#if HAVE_SPI4
# if defined(PINID_SPI4_MOSI) && PINID_SPI4_MOSI > 0
#  define IS_SPI4_MOSI_DEF(_p_) (PINID(_p_) == PINID_SPI4_MOSI)
# else
#  define IS_SPI4_MOSI_DEF(_p_) (0)
# endif
# if defined(PINID_SPI4_MOSI_ALT2) && PINID_SPI4_MOSI_ALT2 > 0
#  define IS_SPI4_MOSI_ALT2(_p_) (PINID(_p_) == PINID_SPI4_MOSI_ALT2)
# else
#  define IS_SPI4_MOSI_ALT2(_p_) (0)
# endif
# if defined(PINID_SPI4_MOSI_ALT) && PINID_SPI4_MOSI_ALT > 0
#  define IS_SPI4_MOSI_ALT1(_p_) (PINID(_p_) == PINID_SPI4_MOSI_ALT)
# else
#  define IS_SPI4_MOSI_ALT1(_p_) (0)
# endif
# define IS_SPI4_MOSI(_p_) (IS_SPI4_MOSI_DEF(_p_) || IS_SPI4_MOSI_ALT1(_p_) || IS_SPI4_MOSI_ALT2(_p_))

# if defined(PINID_SPI4_MISO) && PINID_SPI4_MISO > 0
#  define IS_SPI4_MISO_DEF(_p_) (PINID(_p_) == PINID_SPI4_MISO)
# else
#  define IS_SPI4_MISO_DEF(_p_) (0)
# endif
# if defined(PINID_SPI4_MISO_ALT2) && PINID_SPI4_MISO_ALT2 > 0
#  define IS_SPI4_MISO_ALT2(_p_) (PINID(_p_) == PINID_SPI4_MISO_ALT2)
# else
#  define IS_SPI4_MISO_ALT2(_p_) (0)
# endif
# if defined(PINID_SPI4_MISO_ALT) && PINID_SPI4_MISO_ALT > 0
#  define IS_SPI4_MISO_ALT1(_p_) (PINID(_p_) == PINID_SPI4_MISO_ALT)
# else
#  define IS_SPI4_MISO_ALT1(_p_) (0)
# endif
# define IS_SPI4_MISO(_p_) (IS_SPI4_MISO_DEF(_p_) || IS_SPI4_MISO_ALT1(_p_) || IS_SPI4_MISO_ALT2(_p_))

# if defined(PINID_SPI4_SCK) && PINID_SPI4_SCK > 0
#  define IS_SPI4_SCK_DEF(_p_) (PINID(_p_) == PINID_SPI4_SCK)
# else
#  define IS_SPI4_SCK_DEF(_p_) (0)
# endif
# if defined(PINID_SPI4_SCK_ALT2) && PINID_SPI4_SCK_ALT2 > 0
#  define IS_SPI4_SCK_ALT2(_p_) (PINID(_p_) == PINID_SPI4_SCK_ALT2)
# else
#  define IS_SPI4_SCK_ALT2(_p_) (0)
# endif
# if defined(PINID_SPI4_SCK_ALT) && PINID_SPI4_SCK_ALT > 0
#  define IS_SPI4_SCK_ALT1(_p_) (PINID(_p_) == PINID_SPI4_SCK_ALT)
# else
#  define IS_SPI4_SCK_ALT1(_p_) (0)
# endif
# define IS_SPI4_SCK(_p_) (IS_SPI4_SCK_DEF(_p_) || IS_SPI4_SCK_ALT1(_p_) || IS_SPI4_SCK_ALT2(_p_))

# define IS_SPI4(_mosi_, _miso_, _sck_) (IS_SPI4_MOSI(_mosi_) && IS_SPI4_MISO(_miso_) && IS_SPI4_SCK(_sck_))

# if !defined(SPIx) && IS_SPI4(SPI_MOSI_PIN, SPI_MISO_PIN, SPI_SCK_PIN)
#  define SPIx SPI4
#  define SPIx_CLOCKEN RCC_PERIPH_SPI4
#  define SPIx_AF GPIOAF_SPI4
# endif

#else // HAVE_SPI4
# define IS_SPI4(_mosi_, _miso_, _sck_) (0)
#endif // HAVE_SPI4

//
// SPI5
//
#if defined(SPI5)
# define HAVE_SPI5 1
#else
# define HAVE_SPI5 0
#endif

#if HAVE_SPI5
# if defined(PINID_SPI5_MOSI) && PINID_SPI5_MOSI > 0
#  define IS_SPI5_MOSI_DEF(_p_) (PINID(_p_) == PINID_SPI5_MOSI)
# else
#  define IS_SPI5_MOSI_DEF(_p_) (0)
# endif
# if defined(PINID_SPI5_MOSI_ALT2) && PINID_SPI5_MOSI_ALT2 > 0
#  define IS_SPI5_MOSI_ALT2(_p_) (PINID(_p_) == PINID_SPI5_MOSI_ALT2)
# else
#  define IS_SPI5_MOSI_ALT2(_p_) (0)
# endif
# if defined(PINID_SPI5_MOSI_ALT) && PINID_SPI5_MOSI_ALT > 0
#  define IS_SPI5_MOSI_ALT1(_p_) (PINID(_p_) == PINID_SPI5_MOSI_ALT)
# else
#  define IS_SPI5_MOSI_ALT1(_p_) (0)
# endif
# define IS_SPI5_MOSI(_p_) (IS_SPI5_MOSI_DEF(_p_) || IS_SPI5_MOSI_ALT1(_p_) || IS_SPI5_MOSI_ALT2(_p_))

# if defined(PINID_SPI5_MISO) && PINID_SPI5_MISO > 0
#  define IS_SPI5_MISO_DEF(_p_) (PINID(_p_) == PINID_SPI5_MISO)
# else
#  define IS_SPI5_MISO_DEF(_p_) (0)
# endif
# if defined(PINID_SPI5_MISO_ALT2) && PINID_SPI5_MISO_ALT2 > 0
#  define IS_SPI5_MISO_ALT2(_p_) (PINID(_p_) == PINID_SPI5_MISO_ALT2)
# else
#  define IS_SPI5_MISO_ALT2(_p_) (0)
# endif
# if defined(PINID_SPI5_MISO_ALT) && PINID_SPI5_MISO_ALT > 0
#  define IS_SPI5_MISO_ALT1(_p_) (PINID(_p_) == PINID_SPI5_MISO_ALT)
# else
#  define IS_SPI5_MISO_ALT1(_p_) (0)
# endif
# define IS_SPI5_MISO(_p_) (IS_SPI5_MISO_DEF(_p_) || IS_SPI5_MISO_ALT1(_p_) || IS_SPI5_MISO_ALT2(_p_))

# if defined(PINID_SPI5_SCK) && PINID_SPI5_SCK > 0
#  define IS_SPI5_SCK_DEF(_p_) (PINID(_p_) == PINID_SPI5_SCK)
# else
#  define IS_SPI5_SCK_DEF(_p_) (0)
# endif
# if defined(PINID_SPI5_SCK_ALT2) && PINID_SPI5_SCK_ALT2 > 0
#  define IS_SPI5_SCK_ALT2(_p_) (PINID(_p_) == PINID_SPI5_SCK_ALT2)
# else
#  define IS_SPI5_SCK_ALT2(_p_) (0)
# endif
# if defined(PINID_SPI5_SCK_ALT) && PINID_SPI5_SCK_ALT > 0
#  define IS_SPI5_SCK_ALT1(_p_) (PINID(_p_) == PINID_SPI5_SCK_ALT)
# else
#  define IS_SPI5_SCK_ALT1(_p_) (0)
# endif
# define IS_SPI5_SCK(_p_) (IS_SPI5_SCK_DEF(_p_) || IS_SPI5_SCK_ALT1(_p_) || IS_SPI5_SCK_ALT2(_p_))

# define IS_SPI5(_mosi_, _miso_, _sck_) (IS_SPI5_MOSI(_mosi_) && IS_SPI5_MISO(_miso_) && IS_SPI5_SCK(_sck_))

# if !defined(SPIx) && IS_SPI5(SPI_MOSI_PIN, SPI_MISO_PIN, SPI_SCK_PIN)
#  define SPIx SPI5
#  define SPIx_CLOCKEN RCC_PERIPH_SPI5
#  define SPIx_AF GPIOAF_SPI5
# endif

#else // HAVE_SPI5
# define IS_SPI5(_mosi_, _miso_, _sck_) (0)
#endif // HAVE_SPI5

//
// SPI6
//
#if defined(SPI6)
# define HAVE_SPI6 1
#else
# define HAVE_SPI6 0
#endif

#if HAVE_SPI6
# if defined(PINID_SPI6_MOSI) && PINID_SPI6_MOSI > 0
#  define IS_SPI6_MOSI_DEF(_p_) (PINID(_p_) == PINID_SPI6_MOSI)
# else
#  define IS_SPI6_MOSI_DEF(_p_) (0)
# endif
# if defined(PINID_SPI6_MOSI_ALT2) && PINID_SPI6_MOSI_ALT2 > 0
#  define IS_SPI6_MOSI_ALT2(_p_) (PINID(_p_) == PINID_SPI6_MOSI_ALT2)
# else
#  define IS_SPI6_MOSI_ALT2(_p_) (0)
# endif
# if defined(PINID_SPI6_MOSI_ALT) && PINID_SPI6_MOSI_ALT > 0
#  define IS_SPI6_MOSI_ALT1(_p_) (PINID(_p_) == PINID_SPI6_MOSI_ALT)
# else
#  define IS_SPI6_MOSI_ALT1(_p_) (0)
# endif
# define IS_SPI6_MOSI(_p_) (IS_SPI6_MOSI_DEF(_p_) || IS_SPI6_MOSI_ALT1(_p_) || IS_SPI6_MOSI_ALT2(_p_))

# if defined(PINID_SPI6_MISO) && PINID_SPI6_MISO > 0
#  define IS_SPI6_MISO_DEF(_p_) (PINID(_p_) == PINID_SPI6_MISO)
# else
#  define IS_SPI6_MISO_DEF(_p_) (0)
# endif
# if defined(PINID_SPI6_MISO_ALT2) && PINID_SPI6_MISO_ALT2 > 0
#  define IS_SPI6_MISO_ALT2(_p_) (PINID(_p_) == PINID_SPI6_MISO_ALT2)
# else
#  define IS_SPI6_MISO_ALT2(_p_) (0)
# endif
# if defined(PINID_SPI6_MISO_ALT) && PINID_SPI6_MISO_ALT > 0
#  define IS_SPI6_MISO_ALT1(_p_) (PINID(_p_) == PINID_SPI6_MISO_ALT)
# else
#  define IS_SPI6_MISO_ALT1(_p_) (0)
# endif
# define IS_SPI6_MISO(_p_) (IS_SPI6_MISO_DEF(_p_) || IS_SPI6_MISO_ALT1(_p_) || IS_SPI6_MISO_ALT2(_p_))

# if defined(PINID_SPI6_SCK) && PINID_SPI6_SCK > 0
#  define IS_SPI6_SCK_DEF(_p_) (PINID(_p_) == PINID_SPI6_SCK)
# else
#  define IS_SPI6_SCK_DEF(_p_) (0)
# endif
# if defined(PINID_SPI6_SCK_ALT2) && PINID_SPI6_SCK_ALT2 > 0
#  define IS_SPI6_SCK_ALT2(_p_) (PINID(_p_) == PINID_SPI6_SCK_ALT2)
# else
#  define IS_SPI6_SCK_ALT2(_p_) (0)
# endif
# if defined(PINID_SPI6_SCK_ALT) && PINID_SPI6_SCK_ALT > 0
#  define IS_SPI6_SCK_ALT1(_p_) (PINID(_p_) == PINID_SPI6_SCK_ALT)
# else
#  define IS_SPI6_SCK_ALT1(_p_) (0)
# endif
# define IS_SPI6_SCK(_p_) (IS_SPI6_SCK_DEF(_p_) || IS_SPI6_SCK_ALT1(_p_) || IS_SPI6_SCK_ALT2(_p_))

# define IS_SPI6(_mosi_, _miso_, _sck_) (IS_SPI6_MOSI(_mosi_) && IS_SPI6_MISO(_miso_) && IS_SPI6_SCK(_sck_))

# if !defined(SPIx) && IS_SPI6(SPI_MOSI_PIN, SPI_MISO_PIN, SPI_SCK_PIN)
#  define SPIx SPI6
#  define SPIx_CLOCKEN RCC_PERIPH_SPI6
#  define SPIx_AF GPIOAF_SPI6
# endif

#else // HAVE_SPI6
# define IS_SPI6(_mosi_, _miso_, _sck_) (0)
#endif // HAVE_SPI6

#endif // INCLUDED_BY_SPI_C
