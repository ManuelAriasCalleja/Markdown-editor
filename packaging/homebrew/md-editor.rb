cask "md-editor" do
  version "2.8.0"
  sha256 "a05f6d11d8ed96b8129fad4ed87520f7177e26d4b2a8d0a972e918de2c20f5b1"

  url "https://github.com/ManuelAriasCalleja/Markdown-editor/releases/download/v#{version}/md-editor-#{version}-macos-universal.dmg",
      verified: "github.com/ManuelAriasCalleja/Markdown-editor/"
  name "md-editor"
  desc "WYSIWYG Markdown editor built with Qt6"
  homepage "https://github.com/ManuelAriasCalleja/Markdown-editor"

  # La versión sale de la última release de GitHub; el DMG es universal
  # (arm64 + x86_64), así que un solo cask vale para ambos.
  livecheck do
    url :url
    strategy :github_latest
  end

  app "md-editor.app"

  # El binario NO está firmado ni notarizado: Gatekeeper lo bloqueará la primera
  # vez. Esto se lo explicamos al usuario.
  caveats <<~EOS
    md-editor no está firmado ni notarizado por Apple, así que macOS lo bloqueará
    al abrirlo por primera vez. Para permitirlo:

      • Botón derecho sobre md-editor en Aplicaciones → "Abrir" → "Abrir", o
      • Ajustes del Sistema → Privacidad y seguridad → "Abrir de todas formas".

    Solo hay que hacerlo una vez. Como alternativa por terminal:
      xattr -dr com.apple.quarantine "/Applications/md-editor.app"
  EOS

  zap trash: [
    "~/Library/Preferences/md-editor*.plist",
    "~/Library/Saved Application State/org.mdeditor.app.savedState",
  ]
end
