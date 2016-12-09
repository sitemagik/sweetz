#ifndef sweetcoinADDRESSVALIDATOR_H
#define sweetcoinADDRESSVALIDATOR_H

#include <QRegExpValidator>

/** Base48 entry widget validator.
   Corrects near-miss characters and refuses characters that are no part of base48.
 */
class sweetcoinAddressValidator : public QValidator
{
    Q_OBJECT
public:
    explicit sweetcoinAddressValidator(QObject *parent = 0);

    State validate(QString &input, int &pos) const;

    static const int MaxAddressLength = 35;
signals:

public slots:

};

#endif // sweetcoinADDRESSVALIDATOR_H
