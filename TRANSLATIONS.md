# Traducciones (fork con i18n)

Este fork agrega soporte de traducción al shell de Caelestia, que upstream
todavía no tiene (los `.qml` ya usan `qsTr()` pero no había
infraestructura para cargar traducciones). El primer idioma agregado es
español neutro LATAM, con `es_CL` como scaffolding para overrides
chilenos.

Repo: <https://github.com/JEPEREZR/shell>
Rama: `feat/i18n-es`
Upstream: <https://github.com/caelestia-dots/shell> (issue relacionado:
[#1006 i18n support](https://github.com/caelestia-dots/shell/issues/1006))

## Estado actual

- Infraestructura completa: `QTranslator` se carga al iniciar el plugin
  `Caelestia` y resuelve el idioma desde `QLocale::system()` con
  fallbacks (`es_CL` → `es`).
- `caelestia_es.ts`: 665/665 strings traducidos.
- `caelestia_es_CL.ts`: vacío. Existe solo como scaffolding; cuando
  haya una palabra que no suene chilena, se agrega override aquí.
- Paquete instalado: `caelestia-shell-es` (conflicta y reemplaza al
  `caelestia-shell` oficial).

## Estructura

```
plugin/src/Caelestia/translations.cpp  # loader del QTranslator (entry point)
plugin/src/Caelestia/CMakeLists.txt    # registra translations.cpp como source
translations/
  CMakeLists.txt                       # genera .qm con lrelease6, los instala
  caelestia_es.ts                      # source XML (Qt Linguist)
  caelestia_es_CL.ts                   # scaffolding regional
  apply_es.py                          # script con el diccionario es completo
CMakeLists.txt                         # añadido módulo "translations"
```

Los `.qm` se instalan en `/usr/share/caelestia-shell/translations/`.
`translations.cpp` los busca ahí (y en algunos paths estándar de XDG).

## Cómo funciona en runtime

1. Quickshell carga `shell.qml` desde `/etc/xdg/quickshell/caelestia/`.
2. `shell.qml` importa módulos que a su vez importan `Caelestia` (el
   plugin C++).
3. Al cargarse el plugin, `Q_COREAPP_STARTUP_FUNCTION` corre
   `installCaelestiaTranslator()`.
4. La función lee `QLocale::system().uiLanguages()`, prueba cada
   candidato (`es_CL` → `es`), carga el `.qm` correspondiente y lo
   registra con `QCoreApplication::installTranslator()`.
5. Qt emite `LanguageChange` y los `qsTr()` en QML retraducen.

## Cómo agregar/cambiar traducciones

### Editar una traducción puntual

1. Abre `translations/apply_es.py` y modifica la entrada en el dict
   `TRANSLATIONS`.
2. `python3 translations/apply_es.py` (reescribe `caelestia_es.ts`).
3. Reconstruir e instalar (ver abajo).

### Re-extraer strings tras cambios en `.qml`

Cuando upstream agrega nuevos `qsTr(...)` al rebasar con `main`:

```bash
cd ~/proyectos/caelestia-shell
lupdate6 -recursive -extensions qml,js -no-obsolete -locations none \
    -source-language en -target-language es \
    shell.qml components modules services utils \
    -ts translations/caelestia_es.ts
python3 translations/apply_es.py   # rellena las nuevas
```

`apply_es.py` reporta cuáles strings nuevos están sin traducir; se
agregan al dict y se vuelve a correr.

### Override chileno

Editar `translations/caelestia_es_CL.ts` y poner traducciones para los
strings que difieran del español neutro. Qt usa esa traducción primero
cuando `LANG=es_CL.*` y cae a `caelestia_es.qm` para todo lo demás.

## Reconstruir e instalar

PKGBUILD está fuera del repo, en `~/proyectos/pkg-caelestia-shell-es/`.
Construye desde el clone local (`source=git+file://...`), así toma
únicamente lo que esté **committeado** en la rama `feat/i18n-es`.

```bash
cd ~/proyectos/caelestia-shell
git add -u && git commit -m "..."

cd ~/proyectos/pkg-caelestia-shell-es
makepkg -si --noconfirm    # construye e instala
```

Para reiniciar el shell y ver el cambio (depende de cómo lo lances):
`pkill quickshell` y volver a abrirlo, o reiniciar la sesión Hyprland.

## Volver al upstream

Si en algún momento se quiere desinstalar el fork y volver al paquete
oficial:

```bash
sudo pacman -Rns caelestia-shell-es
# si tu instalación original era del AUR:
paru -S caelestia-shell-git
```

## Pendientes

- Validar visualmente que toda la UI esté traducida (algunos strings
  difíciles de detectar via grep — diálogos modales, errores, toasts).
- Decidir si vale la pena mandar la infra al upstream (issue #1006). El
  fork es personal hoy.
- Mantenerse al día con `main` del upstream: cuando agreguen `qsTr()`
  nuevos, re-correr `lupdate6` + `apply_es.py` + rebuild.
