{lib, config, pkgs, ...}:
let
  artempd = (pkgs.callPackage ./default.nix {});
in 
{
  options = {
    services.artemp.enable = lib.mkEnableOption "enable arduino module";
  };
  config = lib.mkIf config.services.artemp.enable {
    environment.systemPackages = [
      artempd
    ];
    systemd.services."artempd@" = {
      description = "Arduino temperature display service for %I";
      path = [ pkgs.coreutils ];
      serviceConfig = {
        Type = "simple";
        ExecStart = "${artempd}/bin/artempd /dev/%I";
      };
    };
    services.udev.extraRules = ''
    KERNEL=="ttyUSB*", TAG+="systemd", ACTION=="bind", ENV{SYSTEMD_WANTS}+="artempd@%k.service"
    '';
  };
}
