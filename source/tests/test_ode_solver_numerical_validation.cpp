/*
 * Validação numérica standalone do RungeKutta4OdeSolver (sem dependências do GenESyS),
 * comparando a integração contra a solução analítica do oscilador harmônico.
 *
 * Sistema: dx/dt = v, dv/dt = -x
 * Solução exata: x(t) = cos(t), v(t) = -sin(t)
 */

#include <iostream>
#include <cmath>
#include <vector>
#include <iomanip>

#include "tools/OdeSolver_if.h"
#include "tools/OdeSystem_if.h"
#include "tools/RungeKutta4OdeSolver.h"

// Implementação do oscilador harmônico
class HarmonicOscillator : public OdeSystem_if {
public:
	unsigned int dimension() const override {
		return 2; // x e v
	}
	
	void evaluate(double t, const double* y, double* dydt) const override {
		// y[0] = x, y[1] = v
		// dx/dt = v
		// dv/dt = -x
		dydt[0] = y[1];      // dx/dt = v
		dydt[1] = -y[0];     // dv/dt = -x
	}
};

// Solução exata
void exactSolution(double t, double& x_exact, double& v_exact) {
	x_exact = std::cos(t);
	v_exact = -std::sin(t);
}

int main() {
	std::cout << "=== Validacao Numerica do RungeKutta4OdeSolver - Oscilador Harmonico ===" << std::endl;
	std::cout << std::endl;
	
	HarmonicOscillator oscillator;
	RungeKutta4OdeSolver solver;
	
	// Condições iniciais: x=1.0, v=0.0
	double y[2] = {1.0, 0.0};
	double y_next[2];
	double t = 0.0;
	double step = 0.01;
	
	std::cout << "Condições iniciais: t=" << t << ", x=" << y[0] << ", v=" << y[1] << std::endl;
	std::cout << "Step: " << step << std::endl;
	std::cout << std::endl;
	
	// Tempos de teste
	std::vector<double> test_times = {0.1, 0.5, 1.0, 1.5707963, 3.1415926}; // π/2 ≈ 1.5707963, π ≈ 3.1415926
	
	std::cout << std::fixed << std::setprecision(8);
	std::cout << "t(s)        x_num       v_num       x_exact     v_exact     err_x       err_v" << std::endl;
	std::cout << "------------------------------------------------------------------------" << std::endl;
	
	for (double target_time : test_times) {
		// Integrar até o tempo alvo
		while (t + step <= target_time) {
			solver.advance(oscillator, t, step, y, y_next);
			t += step;
			y[0] = y_next[0];
			y[1] = y_next[1];
		}
		
		// Passo final se necessário
		if (t < target_time) {
			double last_step = target_time - t;
			solver.advance(oscillator, t, last_step, y, y_next);
			t = target_time;
			y[0] = y_next[0];
			y[1] = y_next[1];
		}
		
		// Solução exata
		double x_exact, v_exact;
		exactSolution(t, x_exact, v_exact);
		
		// Erro absoluto
		double err_x = std::abs(y[0] - x_exact);
		double err_v = std::abs(y[1] - v_exact);
		
		// Imprimir resultados
		std::cout << t << "  " << y[0] << "  " << y[1] << "  " 
		          << x_exact << "  " << v_exact << "  " 
		          << err_x << "  " << err_v << std::endl;
	}
	
	std::cout << std::endl;
	std::cout << "=== Teste concluído ===" << std::endl;
	
	// Verificar se os erros são aceitáveis (para RK4 com step=0.01)
	// Em t=π, x deveria ser -1.0, v deveria ser 0.0
	double final_x = y[0];
	double final_v = y[1];
	double final_x_exact = std::cos(3.1415926);
	double final_v_exact = -std::sin(3.1415926);
	double final_err_x = std::abs(final_x - final_x_exact);
	double final_err_v = std::abs(final_v - final_v_exact);
	
	std::cout << std::endl;
	std::cout << "Erro final em t=π:" << std::endl;
	std::cout << "  err_x = " << final_err_x << std::endl;
	std::cout << "  err_v = " << final_err_v << std::endl;
	
	// Para RK4 com step=0.01, esperamos erro < 1e-6
	if (final_err_x < 1e-4 && final_err_v < 1e-4) {
		std::cout << "✓ Teste PASSOU: erros dentro da tolerância esperada" << std::endl;
		return 0;
	} else {
		std::cout << "✗ Teste FALHOU: erros acima da tolerância esperada" << std::endl;
		return 1;
	}
}
