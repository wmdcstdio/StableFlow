#pragma once

#include "gridmath.h"

Float Grid_Norm(const Grid &G) {
	MatrixXd M = G;
	return M.lpNorm<2>();
}

template<typename T>
T Clip(T a, T mn, T mx) {
	a = max(a, mn);
	a = min(a, mx);
	return a;
}

void ConstMask::resize(const Grid &A){
	int n = A.rows();
	int m = A.cols();
	dlt.resize(n, m);
	dlt.setZero();
	msk.resize(n, m);
	msk.setZero();
}

void ConstMask::Set_Box(Float x0, Float x1, Float y0, Float y1, Float d) {
	int n = dlt.rows(), m = dlt.cols();
	int i0 = Clip(int(x0*n), 0, n - 1);
	int i1 = Clip(int(x1*n), 0, n - 1);
	int j0 = Clip(int(y0*n), 0, m - 1);
	int j1 = Clip(int(y1*n), 0, m - 1);
	for (int i = i0; i <= i1; i++) {
		for (int j = j0; j <= j1; j++) {
			msk(i, j) = 1;
			dlt(i, j) = d;
		}
	}
}

void ConstMask::Set_Ellipse(Float x0, Float y0, Float a, Float b, Float d){
	int n = dlt.rows(), m = dlt.cols();
	for (int i = 0; i < n; i++) {
		for (int j = 0; j < m; j++) {
			Float x1 = (i + 0.0) / n, y1 = (j + 0.0) / m;
			Float dx = x1 - x0, dy = y1 - y0;
			if (dx*dx*b*b + dy * dy*a*a <= a * a*b*b) {
				msk(i, j) = 1;
				dlt(i, j) = d;
			}
		}
	}
}

void ConstMask::Set_Real_Circle(Float x0, Float y0, Float rh, Float d) {
	Set_Ellipse(x0, y0, rh, rh / WHRATIO, d);
}

void Apply_ConstMask(Grid &A, const ConstMask &B) {
	int n = A.rows(), m = A.cols();
	Assert(B.dlt.rows() == n && B.dlt.cols() == m, "ConstBlock size not match");
	for (int i = 0; i < n; i++) {
		for (int j = 0; j < m; j++) {
			if (B.msk(i, j) == 1) {
				A(i, j) = B.dlt(i, j);
			}
		}
	}
}



void Truncate_Index(const Grid &A, int &i, int &j) {
	int n = A.rows(), m = A.cols();
	i = i % n;
	j = j % m;
	if (i < 0) i += n;
	if (j < 0) j += m;
}

void Truncate_Position(const Grid &A, Float &x, Float &y) {
	int n = A.rows(), m = A.cols();
	if (x < 0.5) x = 0.5;
	if (x > n - 1.5) x = n - 1.5;
	if (y < 0.5) y = 0.5;
	if (y > m - 1.5) y = m - 1.5;
}

/*void Add_Block(Grid &A, Float x0, Float x1, Float y0, Float y1, Float d) {
	int n = A.rows(), m = A.cols();
	int i0 = Clip(int(x0*(n - 2)), 1, n - 2);
	int i1 = Clip(int(x1*(n - 2)), 1, n - 2);
	int j0 = Clip(int(y0*(m - 2)), 1, m - 2);
	int j1 = Clip(int(y1*(m - 2)), 1, m - 2);
	cout << "add block: " << i0 << " " << i1 << " " << j0 << " " << j1 << endl;
	
}*/

// Interpolate with respect to mere array index.
Float Interpolate(const Grid &A, Float x, Float y) {
	int xi1, yi1, xi2, yi2;

	xi1 = floor(x), xi2 = xi1 + 1;
	yi1 = floor(y), yi2 = yi1 + 1;
	Float xs = x - xi1, ys = y - yi1;

	int n = A.rows(), m = A.cols();

	Assert(0 <= xi1 && xi1 < n, "xi1 out of range");
	Assert(0 <= xi2 && xi2 < n, "xi2 out of range");
	Assert(0 <= yi1 && yi1 < m, "yi1 out of range");
	Assert(0 <= yi2 && yi2 < m, "yi2 out of range");

	Float ret= A(xi1, yi1)*(1 - xs)*(1 - ys) +
		A(xi1, yi2)*(1 - xs)*ys +
		A(xi2, yi1)*xs*(1 - ys) +
		A(xi2, yi2)*xs*ys;
	if (isnan(ret)) {
		cout << A << endl;
	}
	Assert(!isnan(ret), "return value is nan");

	return ret;
}

Grid Grid_D(const Grid &A, AXIS ax, DIFFTYPE typ) {
	int n = A.rows(), m = A.cols();
	Grid ret(n, m);
	if (typ == FORWARD) {
		if (ax == X) {//(A(x+1,y)-A(x,y))/DX
			ret.topRows(n - 1) = (A.bottomRows(n - 1) - A.topRows(n - 1)) / DX;
			ret.bottomRows(1) = (A.topRows(1) - A.bottomRows(1)) / DX;
		}
		else if (ax == Y) {//(A(x,y+1)-A(x,y))/DY
			ret.leftCols(m - 1) = (A.rightCols(m - 1) - A.leftCols(m - 1)) / DY;
			ret.rightCols(1) = (A.leftCols(1) - A.rightCols(1)) / DY;
		}
	}
	else if (typ == BACKWARD) {
		if (ax == X) {//(A(x,y)-A(x-1,y))/DX
			ret.bottomRows(n - 1) = (A.bottomRows(n - 1) - A.topRows(n - 1)) / DX;
			ret.topRows(1) = (A.topRows(1) - A.bottomRows(1)) / DX;
		}
		else if (ax == Y) {//A(x,y)-A(x,y-1)/DY
			ret.rightCols(m - 1) = (A.rightCols(m - 1) - A.leftCols(m - 1)) / DY;
			ret.leftCols(1) = (A.leftCols(1) - A.rightCols(1)) / DY;
		}
	}
	else if (typ == CENTER) {
		if (ax == X) {//(A(x+1,y)-A(x-1,y))/2DX
			ret.block(1, 0, n - 2, m) = (A.bottomRows(n - 2) - A.topRows(n - 2)) / (2 * DX);
			ret.row(0) = (A.row(1) - A.bottomRows(1)) / (2 * DX);
			ret.bottomRows(1) = (A.row(0) - A.row(n - 2)) / (2 * DX);
		}
		else if (ax == Y) {//A(x,y+1)-A(x,y-1)/2DY
			ret.block(0, 1, n, m - 2) = (A.rightCols(m - 2) - A.leftCols(m - 2)) / (2 * DY);
			ret.col(0) = (A.col(1) - A.rightCols(1)) / (2 * DY);
			ret.rightCols(1) = (A.col(0) - A.col(m - 1)) / (2 * DY);
		}
	}
	return ret;
}

Grid Grid_S(const Grid &U, const Grid &V, const Grid &phi) {
	int n = phi.rows(), m = phi.cols();
	Grid ret = U * Grid_D(phi, X, CENTER) + V * Grid_D(phi, Y, CENTER) + Grid_D(U*phi, X, CENTER) + Grid_D(V*phi, Y, CENTER);
	return ret / 2.0;
}


/*Float Local_Diff(Matrix A, int x, int y, int order, AXIS ax, DIFFTYPE typ)
{
	Assert(0 <= x, "in Local_Diff,x<0");
	Assert(x < A.rows(), "in Local_Diff, x exceed rows");
	Assert(0 <= y, "in Local_Diff,y<0");
	Assert(y < A.cols(), "in Local_Diff, y exceed cols");
	Assert(order == 1, "in Local_Diff, order!=1");

	if (order == 1) {
		if (typ == FORWARD) {
			if (ax == X) {
				if (x + 1 < A.rows()) {
					return (A(x + 1, y) - A(x, y)) / DX;
				}
				else {
					return (A(0, y) - A(x, y)) / DX;
				}
			}
			else if (ax == Y) {
				if (y + 1 < A.cols()) {
					return (A(x, y + 1) - A(x, y)) / DY;
				}
				else {
					return (A(x, 0) - A(x, y)) / DY;
				}
			}
		}
		else if (typ == BACKWARD) {
			if (ax == X) {
				if (x > 0) {
					return (A(x, y) - A(x - 1, y)) / DX;
				}
				else {
					return (A())
					if (x != A.rows() - 1) {
						printf("Diff error: wrong coordinate (%d %d)\n", x, y);
						throw x;
					}
					else {
						return (A(0, y) - A(x, y)) / DX;
					}
				}
			}
		}
	}
	else {
		printf("Diff error: wrong order %d\n", order);
		throw order;
	}
}*/


