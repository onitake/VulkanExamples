#!/usr/bin/perl

use strict;
use warnings;
use IO::File;

my $infile = $ARGV[0] or die("Usage: tex2dds.pl <input.tex> <output.dds>");
my $outfile = $ARGV[1] or die("Usage: tex2dds.pl <input.tex> <output.dds>");

my $in = IO::File->new($infile, '<') or die("Can't open input");
my $out = IO::File->new($outfile, '>') or die("Can't open output");

$in->binmode();
$out->binmode();

my $consumed = 0;

# 01 00 00 00  01 00 00 00  00 00 00 00  48 00 00 00  10 00 10 01  00 0D 01 00  B8 AA AA 00
# \___ver___/  \___ver?__/  \_________/  \_format__/  \/ \_h_/ \____/ \/ \___/  \_datalen_/
# 10 = width / 256
# 0D = MIP levels
# fmt: 48 = DXGI_FORMAT_BC1_UNORM_SRGB, 53 = DXGI_FORMAT_BC5_UNORM
# these come from DXGI_FORMAT (dxgiformat.h)

$in->read(my $buffer, 28);
my ($version, $unknown1, $unknown2, $format, $width8, $height, $unknown3, $miplevels, $unknown4, $datalen) = unpack('(L4CS2CSL)<', $buffer);
my $width = $width8 * 256;
$consumed += 28;

printf("width=%d height=%d format=%d miplevels=%d datalen=%d\n", $width, $height, $format, $miplevels, $datalen);

sub max ($$) { $_[$_[0] < $_[1]] }
sub min ($$) { $_[$_[0] > $_[1]] }
my %COMPRESSED_FORMATS = reverse(
	'DXGI_FORMAT_BC1_TYPELESS'   => 70,
	'DXGI_FORMAT_BC1_UNORM'      => 71,
	'DXGI_FORMAT_BC1_UNORM_SRGB' => 72,
	'DXGI_FORMAT_BC2_TYPELESS'   => 73,
	'DXGI_FORMAT_BC2_UNORM'      => 74,
	'DXGI_FORMAT_BC2_UNORM_SRGB' => 75,
	'DXGI_FORMAT_BC3_TYPELESS'   => 76,
	'DXGI_FORMAT_BC3_UNORM'      => 77,
	'DXGI_FORMAT_BC3_UNORM_SRGB' => 78,
	'DXGI_FORMAT_BC4_TYPELESS'   => 79,
	'DXGI_FORMAT_BC4_UNORM'      => 80,
	'DXGI_FORMAT_BC4_SNORM'      => 81,
	'DXGI_FORMAT_BC5_TYPELESS'   => 82,
	'DXGI_FORMAT_BC5_UNORM'      => 83,
	'DXGI_FORMAT_BC5_SNORM'      => 84,
	'DXGI_FORMAT_BC6H_TYPELESS'  => 94,
	'DXGI_FORMAT_BC6H_UF16'      => 95,
	'DXGI_FORMAT_BC6H_SF16'      => 96,
	'DXGI_FORMAT_BC7_TYPELESS'   => 97,
	'DXGI_FORMAT_BC7_UNORM'      => 98,
	'DXGI_FORMAT_BC7_UNORM_SRGB' => 99,
);
sub iscompressed($) { defined($COMPRESSED_FORMATS{$_[0]}) }
my %PACKED_FORMATS = reverse(
	'DXGI_FORMAT_R8G8_B8G8_UNORM' => 68,
	'DXGI_FORMAT_G8R8_G8B8_UNORM' => 69,
);
sub ispacked($) { defined($PACKED_FORMATS{$_[0]}) }

my $DDS_MAGIC = 0x20534444;
my $DDS_HEADER_SIZE = 124;
my $DDS_HEADER_PIXELFORMAT_SIZE = 32;
my $DDS_HEADER_DXT10_SIZE = 20;

my $DDSD_CAPS = 0x00000001;
my $DDSD_HEIGHT = 0x00000002;
my $DDSD_WIDTH = 0x00000004;
my $DDSD_PITCH = 0x00000008;
my $DDSD_PIXELFORMAT = 0x00001000;
my $DDSD_MIPMAPCOUNT = 0x00020000;
my $DDSD_LINEARSIZE = 0x00080000;
my $DDSD_DEPTH = 0x00800000;
my $DDSCAPS_COMPLEX = 0x8;
my $DDSCAPS_MIPMAP = 0x400000;
my $DDSCAPS_TEXTURE = 0x1000;
my $DDS_HEADER_FLAGS_TEXTURE = $DDSD_CAPS | $DDSD_HEIGHT | $DDSD_WIDTH | $DDSD_PIXELFORMAT ;
my $DDS_HEADER_FLAGS_MIPMAP = $DDSD_MIPMAPCOUNT;
my $DDS_HEADER_FLAGS_VOLUME = $DDSD_DEPTH;
my $DDS_HEADER_FLAGS_PITCH = $DDSD_PITCH;
my $DDS_HEADER_FLAGS_LINEARSIZE = $DDSD_LINEARSIZE;

my $DDPF_ALPHAPIXELS = 0x00000001;
my $DDPF_FOURCC = 0x00000004;
my $DDPF_RGB = 0x00000040;
my $DDPF_RGBA = $DDPF_RGB | $DDPF_ALPHAPIXELS;
my $DDPF_LUMINANCE = 0x00020000;
my $DDPF_LUMINANCEA = $DDPF_LUMINANCE | $DDPF_ALPHAPIXELS;
my $DDPF_ALPHA = 0x00000002;
my $DDPF_PALETTEINDEXED8 = 0x00000020;
my $DDPF_PALETTEINDEXED8A = $DDPF_PALETTEINDEXED8 | $DDPF_ALPHAPIXELS;
my $DDPF_BUMPDUDV = 0x00080000;
my $DDS_DIMENSION_TEXTURE1D = 2;
my $DDS_DIMENSION_TEXTURE2D = 3;
my $DDS_DIMENSION_TEXTURE3D = 4;
my $DDS_RESOURCE_MISC_TEXTURECUBE = 4;
my $DDS_ALPHA_MODE_UNKNOWN = 0;
my $DDS_ALPHA_MODE_STRAIGHT = 1;
my $DDS_ALPHA_MODE_PREMULTIPLIED = 2;
my $DDS_ALPHA_MODE_OPAQUE = 3;
my $DDS_ALPHA_MODE_CUSTOM = 4;

my $flags = $DDSD_CAPS | $DDSD_HEIGHT | $DDSD_WIDTH | $DDSD_PIXELFORMAT;
my $caps = $DDSCAPS_TEXTURE;
if ($miplevels > 0) {
	$flags |= $DDSD_MIPMAPCOUNT;
	$caps |= $DDSCAPS_MIPMAP | $DDSCAPS_COMPLEX;
}
my $pitch;
if (iscompressed($format)) {
	$flags |= $DDSD_LINEARSIZE;
	if ($COMPRESSED_FORMATS{$format} =~ '^DXGI_FORMAT_BC[14]') {
		$pitch = $width * $height / 2;
	} else {
		$pitch = $width * $height;
	}
} else {
	$flags |= $DDSD_PITCH;
	if (ispacked($format)) {
		$pitch = int(($width + 1) / 2) * 4;
	} else {
		$pitch = int(($width * bpp($format) + 7) / 8);
	}
}

print($out pack('(L19)<', $DDS_MAGIC, $DDS_HEADER_SIZE, $flags, $height, $width, $pitch, 0, $miplevels, (0) x 11));
print($out pack('(L2a4L5)<', $DDS_HEADER_PIXELFORMAT_SIZE, $DDPF_FOURCC, 'DX10', (0) x 5));
print($out pack('(L5)<', $caps, 0, 0, 0, 0));
print($out pack('(L5)<', $format, $DDS_DIMENSION_TEXTURE2D, 0, 1, $DDS_ALPHA_MODE_STRAIGHT));

$in->read($buffer, $datalen);
print($out $buffer);

$in->close();
$out->close();