#sed filter to make dvitype output close to ltxpost

1,/^ $/d
/^[[]/d
s/[, ]*hh[:=]*[-0-9][0-9]*//g
s/[, ]*vv[:=]*[-0-9][0-9]*//g
s/ ([0-9]*x[0-9]* pixels)//g
