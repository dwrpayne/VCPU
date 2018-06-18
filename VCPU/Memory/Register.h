
template <unsigned int N>
class RegisterMasked : public Component
{
public:
	RegisterMasked();
	void Connect(const Bundle<N>& data, const Bundle<N>& mask, const Wire& enable);
	void Update();

	const Bundle<N>& Out() const { return out; }

private:
	std::array<DFlipFlopReset, N> bits;
	MultiGate<AndGate, N> writemask;
	Bundle<N> out;
};

template<unsigned int N>
inline RegisterMasked<N>::RegisterMasked()
{
	for (int i = 0; i < N; ++i)
	{
		out.Connect(i, bits[i].Q());
	}
}
template<unsigned int N>
inline void RegisterMasked<N>::Connect(const Bundle<N>& data, const Bundle<N>& mask, const Wire& enable)
{
	writemask.Connect(mask, Bundle<N>(enable));
	for (int i = 0; i < N; ++i)
	{
		bits[i].Connect(data[i], mask[i]);
	}
}


template<unsigned int N>
inline void RegisterMasked<N>::Update()
{
	writemask.Update();
	for (int i = 0; i < N; ++i)
	{
		bits[i].Update();
	}
}
