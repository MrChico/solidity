{nixpkgs ? import <nixpkgs> {}}:
with nixpkgs;
stdenv.mkDerivation {
  name = "solc";
  buildInputs = [
    boost
    cmake
    cvc4
    cln
    ncurses
    python2
    z3
    gmp
  ];
  nativeBuildInputs = [ cmake ];
  hardeningDisable = ["all"];
  cmakeFlags = [
    "-DBoost_USE_STATIC_LIBS=OFF"
  ];
  shellHook = ''
  env LD_PRELOAD=$LD_PRELOAD"./libstdc++"
  '';

}
