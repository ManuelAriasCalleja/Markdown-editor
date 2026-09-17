#ifndef FILEERRORS_H
#define FILEERRORS_H

/// \file
/// \brief Avisos de error de archivo que dicen qué ha pasado y qué hacer.

#include <QString>

class QWidget;

/// Un «No se pudo guardar el archivo: Permission denied» no le dice al usuario qué
/// hacer, y el texto del sistema a menudo ni está en su idioma. Aquí se mira el
/// estado REAL del disco tras el fallo (¿existe la carpeta?, ¿se puede escribir?,
/// ¿queda espacio?) y se traduce a una causa y un remedio. El motivo del sistema se
/// sigue mostrando, pero como detalle. `hint` es comprobable sin GUI
/// (`tst_fileerrors`); `showError` es la presentación común de todos los avisos.
namespace mdfileerr {

/// Qué se intentaba hacer con el archivo.
enum class Op { Read, Write };

/// \brief Causa probable y remedio, traducidos, a partir del estado de `path`.
/// \param path archivo que falló (absoluto).
/// \param op lectura o escritura.
QString hint(const QString &path, Op op);

/// \brief Muestra el aviso: `what` como titular, la pista debajo y, al final, el
/// motivo que dio el sistema (si lo hay).
/// \param parent ventana padre.
/// \param title título de la ventana («No se pudo guardar»…).
/// \param what qué ha fallado, con la ruta ya incluida.
/// \param path archivo, para deducir la pista.
/// \param systemError `errorString()` del sistema; vacío si no hay.
/// \param op lectura o escritura.
void showError(QWidget *parent, const QString &title, const QString &what,
               const QString &path, const QString &systemError, Op op);

}  // namespace mdfileerr

#endif // FILEERRORS_H
