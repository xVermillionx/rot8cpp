# Maintainer: xVermillionx <notvalid@not.valid>

_pkgname="rot8cpp"
# pkgname="${_pkgname}-git"
pkgname="${_pkgname}"
pkgver=r29.215e782
pkgrel=1
epoch=1
pkgdesc="rot8cpp for rotating screen"
url="https://github.com/xVermillionx/rot8cpp"
arch=(x86_64 aarch64)
license=(custom:CLOSED)
provides=("${pkgname}")
conflicts=("${pkgname}")
# provides=("${pkgname%-git}")
# conflicts=("${pkgname%-git}")
depends=()
makedepends=(git cmake jsoncpp)
# optdepends=(
#   "grim: required for the screenshot portal to function"
#   "slurp: support for interactive mode for the screenshot portal; one of the built-in chooser options for the screencast portal"
# )
source=("${_pkgname}::git+https://github.com/xVermillionx/rot8cpp.git")
sha256sums=('SKIP')
options=(!makeflags !buildflags !strip)

pkgver() {
  cd "$_pkgname"
  ( set -o pipefail
    git describe --long 2>/dev/null | sed 's/\([^-]*-g\)/r\1/;s/-/./g' ||
    printf "r%s.%s" "$(git rev-list --count HEAD)" "$(git rev-parse --short HEAD)"
  )
}

build() {
	cd "${srcdir}/${_pkgname}"
  cmake -B build
  make -C build rot8cpp
  cd ..
}

package() {
	cd "${srcdir}/${_pkgname}"
  make -C build rot8cpp
  # make -C build install
  # make install
	install -Dm755 build/rot8cpp -t "${pkgdir}/usr/bin"
	install -Dm644 LICENSE -t "${pkgdir}/usr/share/licenses/${_pkgname}"
}
