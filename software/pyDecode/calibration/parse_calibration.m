clear all

function [] = pes(name, matice, file)

  fprintf(file, ["calibration_", name, " = numpy.array([["]);

  for i=1:65536

    fprintf(file, "%4.10f", matice(i));

    if i ~= 65536

      fprintf(file, ", ");

    else

      fprintf(file, "]])\n");

    end

    if mod(i, 256) == 0

      fprintf(file, "\n");

    end

  end

end

a = load('_calibration_a.txt');
b = load('_calibration_b.txt');
c = load('_calibration_c.txt');
t = load('_calibration_t.txt');

file = fopen("calibration.py", "w");

fprintf(file, "import numpy\n\n");

pes("a", a, file);
pes("b", b, file);
pes("c", c, file);
pes("t", t, file);

fclose(file);

