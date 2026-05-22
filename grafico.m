T=[0:1e-3:100];
X=[0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1];

errRel = @(ev, x) 0.01 + ev / (x*(1-x));

fig = figure();
hold on; grid on;
title('Análisis propagación error relativo del DUT');
xlabel('Error relativo tensión (\epsilon_v)');
ylabel('Error relativo DUT');
ylim([0, 100]);

for (i=1:10)
  plot(T, errRel(T, X(i)), 'LineWidth', 3);
end

legend(
'V_{out} / V_{in} = 0.1',
'V_{out} / V_{in} = 0.2',
'V_{out} / V_{in} = 0.3',
'V_{out} / V_{in} = 0.4',
'V_{out} / V_{in} = 0.5',
'V_{out} / V_{in} = 0.6',
'V_{out} / V_{in} = 0.7',
'V_{out} / V_{in} = 0.8',
'V_{out} / V_{in} = 0.9',
'V_{out} / V_{in} = 1'
);

set(gca, 'fontsize', 16);

print(fig, 'error_rel.png', '-dpng', '-r300');
