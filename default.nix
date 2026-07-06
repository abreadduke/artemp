{pkgs ? import <nixpkgs> {}}:
let
  inherit (pkgs) stdenv;
in 
stdenv.mkDerivation{
  name = "Arduino temperature display";
  src = ./.;
  installPhase = ''
    mkdir -p $out/bin
    cp bin/out $out/bin/artempd
  '';
}
